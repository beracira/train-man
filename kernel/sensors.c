#include "sensors.h"
#include "io.h"
#include "td.h"
#include "courier.h"
#include "terminal_input_handler.h"
#include "path_finding.h"
#include "track.h"
#include "clockserver.h"
#include "velocity.h"
#include "train_ui.h"

#define SENSOR_ARRAY_SIZE 10
#define RUNNING_TRAIN 64

int last_sensor = 0;
int sensor_requested = 0;

int check_missed_switch(int cur_sensor, int time);

void get_sensor_data() {
  int sensors[10];
  int sensor_len = 0;
  char sensor_letter[SENSOR_ARRAY_SIZE];
  int sensor_digit[SENSOR_ARRAY_SIZE];
  int i;
  for (i = 0; i < SENSOR_ARRAY_SIZE; ++i) {
    sensor_letter[i] = 0;
  }
  int tail = 0;
  sensor_requested = 0;

  //////////////////////////// 
  // velocity calibration

  int prev_time = 0;

  int time = 0;

  int prev_sensor = -1;
  volatile track_node * track = (track_node *) 0x01700000;

 // int i = -1;
  int d = 0;

  //////////////////////////// 

  while (1 + 2 == 3) {

    int c = Getc(1);
    sensors[sensor_len++] = c;

    if (sensor_len == 10) {
      int k;
      for (k = 0; k < 5; ++k) {
        int high = sensors[k * 2];
        int low = sensors[k * 2 + 1];
        int radix = 16;
        while (low) {
          if (low & 1) {
            // putchar_terminal('A' + k);
            sensor_letter[tail] = 'A' + k;
            // put_int(radix);
            sensor_digit[tail] = radix;
            tail += 1;
            tail %= SENSOR_ARRAY_SIZE;
            // putchar_terminal(' ');
          }
          low >>= 1;
          radix--;
        }
        radix = 8;
        while (high) {
          if (high & 1) {
            // putchar_terminal('A' + k);
            sensor_letter[tail] = 'A' + k;
            // put_int(radix);
            sensor_digit[tail] = radix;
            tail += 1;
            tail %= SENSOR_ARRAY_SIZE;
            // putchar_terminal(' ');
          }
          high >>= 1;
          radix--;
        }
      }
      printf(2, "\033[s\033[27;30H\033[K");

      int i = (tail - 1 + SENSOR_ARRAY_SIZE) % SENSOR_ARRAY_SIZE;
      last_sensor = (sensor_letter[i] - 'A') * 16;
      last_sensor += sensor_digit[i] - 1;
      int last_sensor2 = (sensor_letter[i] - 'A') * 16 + sensor_digit[i] - 1;

      printf(2, "%d  ", last_sensor);
      do {
        if (sensor_letter[i] != 0)
          printf(2, "%c%d ", sensor_letter[i], sensor_digit[i]);
        i -= 1;
        i += SENSOR_ARRAY_SIZE;
        i %= SENSOR_ARRAY_SIZE;
      } while (i != (tail - 1 + SENSOR_ARRAY_SIZE) % SENSOR_ARRAY_SIZE);

      printf(2, "\033[u");
      sensor_len = 0;

      // if (path_len == -1) {
      //   printf(2, "\033[s\033[10;40H\033[K\033[0;31mNO PATH\033[0m\033[u");
      // } else if (path_len >= 2) {
      //   int flag = 0;
      //   printf(2, "\033[s\033[10;40H\033[K\033[0;31m");
      //   for (i = 0; i < path_len; ++i) {
      //     if (track[path[i]].type == NODE_SENSOR) {
      //       if (flag) {
      //         printf(2, "\033[32m");
      //       }
      //       if (path[i] == last_sensor) {
      //         flag = 1;
      //       }
      //       printf(2, "%s ", track[path[i]].name);
      //     }
      //   }
      //   // if (flag) path_len = 0;
      //   printf(2, "\033[0m\033[u");
      // }

      //////////////////////////// 
      // velocity calibration
    
     if (prev_sensor == -1) {
        prev_sensor = last_sensor2;
        train_64_struct_ptr->prev_sensor = last_sensor2;
        sensor_requested = 0;
        volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
        if (td[CR_TID].state == SENSOR_BLOCKED) td[CR_TID].state = READY;
        if (target_sensor == last_sensor && td[INPUT_TID].state == PATH_SWITCH_BLOCKED) td[INPUT_TID].state = READY;
        continue;
      } else if (prev_sensor == last_sensor2) {
        sensor_requested = 0;
        volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
        if (td[CR_TID].state == SENSOR_BLOCKED) td[CR_TID].state = READY;
        if (target_sensor == last_sensor && td[INPUT_TID].state == PATH_SWITCH_BLOCKED) td[INPUT_TID].state = READY;
        continue;
      }

      if (track[prev_sensor].edge[DIR_AHEAD].dest->type == NODE_BRANCH){
        d = track[prev_sensor].edge[DIR_AHEAD].dest->dir;
      } else {
        d = DIR_AHEAD;
      }

      time = time_ticks - prev_time;
      prev_time = time_ticks;

      int predict_time = 0;
      int predict_dist = get_next_sensor_dist(prev_sensor);
      if (prev_sensor != -1) {
        predict_time = train_velocity[train_64][train_list_ptr[RUNNING_TRAIN]][prev_sensor][last_sensor2];
        if (predict_time == 1) {
          int temp = default_speed[train_64][train_list_ptr[RUNNING_TRAIN]];
          if (temp == -1 || temp == 0) predict_time = 0;
          else predict_time = predict_dist / default_speed[train_64][train_list_ptr[RUNNING_TRAIN]];
        }
      }

      if (predict_time > 1200 || predict_time < 0) predict_time = 0;

      printf(2, "\033[s\033[3;40H\033[K From %s to %s Predicted dist: %d\033[u",
        track[prev_sensor].name, track[last_sensor2].name, predict_dist);
      printf(2, "\033[s\033[4;40H\033[K Predicted time: %d Actual time: %d Delta: %d Dist: %d\033[u", 
        predict_time, time, time - predict_time, (time / predict_dist) * (time - predict_time));
      track_node * temp = get_next_sensor(last_sensor2);
      printf(2, "\033[s\033[5;40H\033[K Next Sensor: %s\033[u", temp != 0 ? temp->name : "NULL");
      update_train_velocity(RUNNING_TRAIN, train_list_ptr[RUNNING_TRAIN], prev_sensor, last_sensor, time);
      prev_sensor = last_sensor2;
        //////////////////////////// 

      /* ---------     missed switches     ----------- */
      check_missed_switch(last_sensor2, time);
      train_64_struct_ptr->prev_sensor = last_sensor2;
      /* --------------------------------------------- */
    }
    sensor_requested = 0;
    volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
    if (td[CR_TID].state == SENSOR_BLOCKED) td[CR_TID].state = READY;
    if (target_sensor == last_sensor && td[INPUT_TID].state == PATH_SWITCH_BLOCKED) td[INPUT_TID].state = READY;
  }
}

track_node * get_next_sensor(int sensor) {
   volatile track_node * track = (track_node *) 0x01700000;

   track_node * k2;
   k2 =  track[sensor].edge[DIR_AHEAD].dest;
   while (k2->type == NODE_BRANCH || k2->type == NODE_MERGE) {
     k2 =  k2->edge[k2->dir].dest;
   }
   return k2;
}

int get_next_sensor_dist(int sensor) {
  volatile track_node * track = (track_node *) 0x01700000;
  int dist = track[sensor].edge[DIR_AHEAD].dist;

  track_node * k2 =  track[sensor].edge[DIR_AHEAD].dest;
  while (k2->type == NODE_BRANCH || k2->type == NODE_MERGE) {
    dist += k2->edge[k2->dir].dist;
    k2 =  k2->edge[k2->dir].dest;
  }
  return dist;
}

int find_path_by_sensor(int train, int speed, int origin, int dest, int trackid, int * path, int len, int time, int total_time) {
  volatile track_node * track = (track_node *) TRACK_ADDR;

  track_node * this = (track_node *) &track[trackid];
  track_node * next = track[trackid].edge[DIR_AHEAD].dest;

  // base case
  if (trackid == dest) {
    path[len] = dest;
    return len;
  }
  if (total_time >= time - len * 10 && total_time <= time + len * 10) {
    path[len - 1] = -1;
    return 0;
  } 
  if (this->type == NODE_EXIT) {
    path[len - 1] = -1;
    return 0;
  }

  // recurse
  if (this->type == NODE_SENSOR) {
    if (trackid != origin && trackid != dest) {
      return -1; // missed sensor
    }

    int t1 = train_velocity[train][speed][origin][next->index];
    int t2 = train_velocity[train][speed][track[origin].reverse->index][next->reverse->index];
    if (t1 == 1) {
      if (t2 == 1) {
        path[len - 1] = -1;
        return 0;
      } else {
        total_time += t2;
      }
    } else {
      total_time += t1;
    }


    path[len] = trackid;
    return find_path_by_sensor(train, speed, next->index, dest, 
                               track[trackid].edge[DIR_AHEAD].dest->index,
                               path, len + 1, time, total_time);

  } else if (this->type == NODE_BRANCH) {
    path[len] = trackid;

    int straight = find_path_by_sensor(train, speed, origin, dest, 
                                       track[trackid].edge[DIR_STRAIGHT].dest->index, 
                                       path, len + 1, time, total_time);

    int curved = find_path_by_sensor(train, speed, origin, dest, 
                                     track[trackid].edge[DIR_CURVED].dest->index, 
                                     path, len + 1 + straight, time, total_time);
    
    if (straight == -1 && curved == -1) {
      return -1;
    } else if (straight == -1) {
      return curved;
    } else if (curved == -1) {
      return straight;
    } else {
      if (straight > curved) {
        int i;
        for (i = len + 1; i < curved - straight + len + 1; i++) {
          path[i] = path[straight + 1];
          path[straight + 1] = -1;
        }
        return i + 1;
      } else {
        return straight;
      }
    }

  } else {
    path[len] = trackid;
    return find_path_by_sensor(train, speed, origin, dest, 
                               track[trackid].edge[DIR_AHEAD].dest->index, 
                               path, len + 1, time, total_time);
  } 
}

int switch_path[200] = {};


int check_missed_switch(int cur_sensor, int time) {

  volatile track_node * track = (track_node *) 0x01700000;

  int prev_sensor = train_64_struct_ptr->prev_sensor;

  int i;
  for (i = 0; i < 100; i++) {
    switch_path[i] = -1;
  }

  int len = find_path_by_sensor(64, 8,  
                                track[cur_sensor].reverse->index, 
                                track[prev_sensor].reverse->index,
                                track[cur_sensor].reverse->index, 
                                switch_path, 0, time, 0);
  if (len <= 0) {
    return 0;
  }

  // iterate backwards through the path
  for (i = len - 1; i >= 0; i--) {
    if (switch_path[i] == -1) {
      break;
    }

    if (track[switch_path[i]].type == NODE_MERGE) {

      // starting point is i
      int x = i;
      int y = i - 1;

      track_node * next_node = track[switch_path[i - 1]].reverse;

      // we move forward in the path until we hit a sensor
      while (next_node->type != NODE_SENSOR) {
        y = y - 1;
        next_node = track[switch_path[y]].reverse;
      }

      // then we move back to the starting point, checking switches along the way
      int z = y;
      while (z != x) {
        track_node * this = track[switch_path[z]].reverse;
        track_node * next = track[switch_path[z + 1]].reverse;

        if (next->type == NODE_BRANCH) {
          int dir;

          if (next->edge[DIR_STRAIGHT].dest->index == this->index) {
            dir = DIR_STRAIGHT;
          } else {
            dir = DIR_CURVED;
          }

          if (next->dir != dir) {
            next->dir = dir;
            printf(2, "\033[s\033[11;40H\033[K\033[0;31mWrong Switch! %s\033[0m\033[u",
               next->name);
            printf(2, "\033[0;35m");
            update_switch(next->num, 33 + dir);
            printf(2, "\033[0m");
          }
        }

        z++;
        // i hate myself
      }
      i = y + 1;
      
    }

  }
  return 0;
}
