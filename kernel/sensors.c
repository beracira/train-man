#include "sensors.h"
#include "io.h"
#include "td.h"
#include "courier.h"
#include "terminal_input_handler.h"
#include "train_ui.h"
#include "path_finding.h"
#include "track.h"
#include "clockserver.h"
#include "velocity.h"

#define SENSOR_ARRAY_SIZE 10
#define RUNNING_TRAIN 64

int last_sensor = 0;
int sensor_requested = 0;


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

  struct train_velocity tv[80];

  int prev_time = 0;

  int time = 0;

  int prev_sensor = -1;
  volatile track_node * track = (track_node *) 0x01700000;

 // int i = -1;
  int d = 0;
  // int y = 5;
  // int k = 0;

  track_node * k2;
  // printf(2, "\033[s");
  for (i = 0; i < 80; i++) {

    tv[i].length = track[i].edge[DIR_AHEAD].dist;

    k2 =  track[i].edge[DIR_AHEAD].dest;
    while (k2->type == NODE_BRANCH || k2->type == NODE_MERGE) {
      tv[i].length += k2->edge[k2->dir].dist;
      k2 =  k2->edge[k2->dir].dest;
    }

    tv[i].num = 0;
    tv[i].time = 0;

  //   if (i < 20) {
  //     printf(2, "\033[%d;30H", i + y);
  //   } else if (i < 40){
  //     printf(2, "\033[%d;65H", i % 20 + y);      
  //   } else if (i < 60){
  //     printf(2, "\033[%d;100H", i % 20 + y);      
  //   } else {
  //     printf(2, "\033[%d;135H", i % 20 + y);      
  //   }
  
  //   printf(2, "%s:%d XX t:%d n:%d sw:%d d:%d", 
  //                 track[i].name, i, 
  //                 tv[i].time, tv[i].num, 2, tv[i].length);
  }
  // printf(2, "\033[u");

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

      if (path_len == -1) {
        printf(2, "\033[s\033[10;40H\033[K\033[0;31mNO PATH\033[0m\033[u");
      } else if (path_len >= 2) {
        int flag = 0;
        printf(2, "\033[s\033[10;40H\033[K\033[0;31m");
        for (i = 0; i < path_len; ++i) {
          if (track[path[i]].type == NODE_SENSOR) {
            if (flag) {
              printf(2, "\033[32m");
            }
            if (path[i] == last_sensor) {
              flag = 1;
            }
            printf(2, "%s ", track[path[i]].name);
          }
        }
        // if (flag) path_len = 0;
        printf(2, "\033[0m\033[u");
      }

      //////////////////////////// 
      // velocity calibration
      if (prev_sensor == -1) {
        prev_sensor = last_sensor2;
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

      /* ---------replace me with something----------- */
      // predict_path(last_sensor2);
      update_train_state(last_sensor2);
      // check_missed_switch(last_sensor2);

      if (track[prev_sensor].edge[DIR_AHEAD].dest->type == NODE_BRANCH){
        d = track[prev_sensor].edge[DIR_AHEAD].dest->dir;
      } else {
        d = DIR_AHEAD;
      }

      time = time_ticks - prev_time;
      prev_time = time_ticks;

      tv[prev_sensor].time = ((tv[prev_sensor].time * tv[prev_sensor].num) + time) 
                            / (tv[prev_sensor].num + 1);
      tv[prev_sensor].num++;

      i = prev_sensor;

      tv[prev_sensor].length = track[prev_sensor].edge[DIR_AHEAD].dist;
      k2 =  track[prev_sensor].edge[DIR_AHEAD].dest;
      while (k2->type == NODE_BRANCH || k2->type == NODE_MERGE) {
        tv[prev_sensor].length += k2->edge[k2->dir].dist;
        k2 =  k2->edge[k2->dir].dest;
      }

      // if (i < 20) {
      //   printf(2, "\033[%d;30H", i + y);
      // } else if (i < 40){
      //   printf(2, "\033[%d;65H", i % 20 + y);      
      // } else if (i < 60){
      //   printf(2, "\033[%d;100H", i % 20 + y);      
      // } else {
      //   printf(2, "\033[%d;135H", i % 20 + y);      
      // }
      // // v = tv[i].length/tv[i].time;
      
      // printf(2, "%s:%d %s t:%d n:%d sw:%d d:%d", 
      //           track[prev_sensor].name, prev_sensor, k2->name, 
      //           tv[prev_sensor].time, tv[prev_sensor].num, d, tv[prev_sensor].length);
      
      // printf(2, "\033[u");

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

void train_init() {
  train_64_struct.prev_sensor = -1;
  train_64_struct.cur_sensor = -1;
  train_64_struct.speed = 0;
  train_64_struct.time_current_sensor = -1;
  train_64_struct.missed_count = 0;
  int i;
  for (i = 0; i < 100; ++i) {
    train_64_struct.predict_sensors[i] = 0;
  }
}

int check_missed_switch(int cur_sensor) {

  volatile track_node * track = (track_node *) 0x01700000;

  track_node * current_node = (track_node *) &track[cur_sensor];
  current_node = current_node->reverse;
  track_node * branch_node = current_node->edge[DIR_AHEAD].dest;
  branch_node = branch_node->reverse;
  if (branch_node->type != NODE_BRANCH) return 1;
  int dir;
  if (branch_node->edge[DIR_STRAIGHT].dest->index == cur_sensor) {
    dir = DIR_STRAIGHT;
  } else {
    dir = DIR_CURVED;
  }

  if (branch_node->dir != dir) {
    branch_node->dir = dir;
    printf(2, "\033[s\033[11;40H\033[K\033[0;31mWrong Switch! %s\033[0m\033[u",
       branch_node->name);
    printf(2, "\033[0;35m");
    update_switch(branch_node->num, 33 + dir);
    printf(2, "\033[0m");
  }

  // train_64_struct.prev_sensor = train_64_struct.cur_sensor;
  return 0;
}

void update_train_state(int sensor) {
  volatile track_node * track = (track_node *) 0x01700000;

  // printf(2, "%d\n\r", train_64_struct.cur_sensor);
  if (train_64_struct.predict_sensors[sensor] || train_64_struct.cur_sensor == -1) {
    train_64_struct.cur_sensor = sensor;
    predict_path(sensor);
    int queue[100];
    int level_queue[100];
    int queue_head = 0;
    int queue_tail = 1;
    int i;
    for (i = 0; i < 100; ++i) {
      train_64_struct.predict_sensors[i] = 0;
      queue[i] = 0;
      level_queue[i] = 0;
    }

    queue[0] = sensor;
    level_queue[0] = 0;
    while (queue_head != queue_tail) {
      // printf(2, "%s\n\r", track[queue[queue_head]].name);
      if (level_queue[queue_head] == 2) {
        queue_head += 1;
        continue;
      }
      track_node * next = get_next_sensor(queue[queue_head]);
      // printf(2, "%s\n\r", next->name);
      if (next->type == NODE_SENSOR) {
        queue[queue_tail] = next->index;
        level_queue[queue_tail] = level_queue[queue_head] + 1;
        queue_tail += 1;
      }
      queue_head += 1;
      continue;
    }
    for (i = 0; i < queue_head; ++i) {
      train_64_struct.predict_sensors[queue[i]] = 1;
    }
    printf(2, "\033[s\033[13;40H\033[KPredicted Sensors: %d ", queue_head);
    for (i = 0; i < 100; ++i) {
      if (train_64_struct.predict_sensors[i] != 0) {
        printf(2, "%s ", track[i].name);
      }
    }
    printf(2, "\033[u");
    train_64_struct.missed_count = 0;
  } else {
    train_64_struct.missed_count += 1;
    if (train_64_struct.missed_count == 3) {
      train_64_struct.cur_sensor = -1;
    }
  }
}

void predict_path(int sensor) {

  volatile track_node * track = (track_node *) 0x01700000;

  int i;
  track_node * cur_sensor = (track_node *) &track[sensor];
  printf(2, "\033[s\033[12;40H\033[KPredicted Path: ");
  for (i = 0; i < 3; ++i) {
    track_node * next_sensor = get_next_sensor(cur_sensor->index);
    // int next_sensor = 1;
    printf(2, "%s ", next_sensor->name);
    if (next_sensor->type == NODE_EXIT) break;
    cur_sensor = next_sensor;
  }
  printf(2, "\033[u");
}
