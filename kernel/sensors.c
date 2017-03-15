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
#include "stop.h"
#include "train_ui.h"

#define SENSOR_ARRAY_SIZE 10
#define RUNNING_TRAIN 64

int last_sensor = 0;
int sensor_requested = 0;

int switch_path[100] = {};
int switch_path_len = 0;

int sensor_is_valid(int train, int cur_sensor, int time);
int check_missed_switch(int len, int cur_sensor, int train);

void get_sensor_data() {
  int sensors[10];
  int sensor_len = 0;
  int sensor_letter[SENSOR_ARRAY_SIZE];
  int sensor_digit[SENSOR_ARRAY_SIZE];
  int i;
  for (i = 0; i < SENSOR_ARRAY_SIZE; ++i) {
    sensor_letter[i] = 0;
    sensor_digit[i] = 0;
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
      if (last_sensor2 < 0 || last_sensor2 > 80) last_sensor2 = -1;

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
    
      if (train_list_ptr[RUNNING_TRAIN] == 0) {
        prev_time = time_ticks;
      }
      if (prev_sensor == -1) {
        prev_sensor = last_sensor2;
        train_64_struct.prev_sensor = last_sensor2;
        sensor_requested = 0;
        prev_time = time_ticks;
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
      /* ---------    noisy sensors     ----------- */
      int result = update_train_state(last_sensor2);

      // if (result) {
      //   sensor_requested = 0;
      //   volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
      //   if (td[CR_TID].state == SENSOR_BLOCKED) td[CR_TID].state = READY;
      //   if (target_sensor == last_sensor && td[INPUT_TID].state == PATH_SWITCH_BLOCKED) td[INPUT_TID].state = READY;
      //   continue;
      // }
      /* --------------------------------------------- */
      // update_train_state(last_sensor2);

      // HARDCODED
      // printf(2, "before len_valid, prev_time: %d\n\r", prev_time);
      // if (wrong_sensor(prev_sensor, last_sensor2)) {

      // }
      int len_valid = sensor_is_valid(64, last_sensor2, time_ticks - prev_time);


      // printf(2, "------prev sensor: %d, last_sensor2: %d, time: %d ticks: %d prev_time: %d len: %d \n\r" , prev_sensor, 
        // last_sensor2, time_ticks - prev_time, time_ticks, prev_time, len_valid);
      // Delay(1);

      if (len_valid == -1) {
        prev_sensor = last_sensor2;
        prev_time = time_ticks;
        train_64_struct.prev_sensor = last_sensor2;
        sensor_requested = 0;
        volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
        if (td[CR_TID].state == SENSOR_BLOCKED) td[CR_TID].state = READY;
        if (target_sensor == last_sensor && td[INPUT_TID].state == PATH_SWITCH_BLOCKED) td[INPUT_TID].state = READY;
        continue;
      } else if (len_valid > 0) {
        /* ---------     missed switches     ----------- */
        check_missed_switch(len_valid, last_sensor2, 64);
        /* --------------------------------------------- */
      } 

      train_64_struct.prev_sensor = last_sensor2;

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
      // check_missed_switch(last_sensor2, time);
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

track_node * get_next_sensor_or_branch(int sensor) {
   volatile track_node * track = (track_node *) 0x01700000;

   track_node * k2;
   k2 =  track[sensor].edge[DIR_AHEAD].dest;
   while (k2->type == NODE_MERGE) {
     k2 = k2->edge[k2->dir].dest;
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
  train_64_struct.cur_sensor = -1;
  train_64_struct.direction = 1;
  train_64_struct.missed_count = 0;
  train_64_struct.prev_sensor = -1;
  train_64_struct.speed = 0;
  train_64_struct.time_current_sensor = -1;
  
  int i;
  for (i = 0; i < 100; ++i) {
    train_64_struct.predict_sensors[i] = 0;
  }
}

int switch_time_index = 0;
int switch_path_time_diff = 0;
int path_found = 0;

int find_path_by_sensor(int train, int speed, int origin, int dest, int trackid, int * path, int len, int time, int total_time) {
  volatile track_node * track = (track_node *) TRACK_ADDR;

  int train_num = train;
  if (train == 64) train = train_64;

  track_node * this = (track_node *) &track[trackid];
  track_node * next = track[trackid].edge[DIR_AHEAD].dest;

  if (this->type == NODE_EXIT) {
    // printf(2, "base case 2\n\r");
    switch_path_len = 0;
    return -1;
  }

  // base case
  if (trackid == dest) {
    path[len] = trackid;

    int t1 = train_velocity[train][speed][origin][trackid];
    int t2 = train_velocity[train][speed][this->reverse->index][track[origin].reverse->index];
    // printf(2, "at dest, trackid: %s, origin: %s, dest: %s, t1: %d, t2: %d \n\r", 
    //   track[trackid].name, track[origin].name, track[dest].name, t1, t2);

    if (t1 == 1) {
      if (t2 == 1) {
        // printf(2, "impossible edge - base case\n\r");
        switch_path_len = 0;
        return -1;
      } else {
        time = time - t2;
      }
    } else {
      time = time - t1;
    }


    if (time < -20 || time > 20) {
      // printf(2, "at dest, time not met but path found - time diff: %d\n\r", time);
      
        switch_path_len = 0;
        return -1;
      // }
    } else {
      // printf(2, "good path found\n\r");
      path_found = 1;
      switch_path_len = len + 1;
      return time;
    }
  }
  if (time < -20) {
      switch_path_len = 0;
      return -1;
    }

  if (this->type == NODE_SENSOR && trackid != origin && trackid != dest) {
    int t1 = train_velocity[train][speed][origin][trackid];
    int t2 = train_velocity[train][speed][this->reverse->index][track[origin].reverse->index];
    // printf(2, "trackid: %s, origin: %s, dest: %s, t1: %d, t2: %d \n\r", 
      // track[trackid].name, track[origin].name, track[dest].name, t1, t2);

    if (t1 == 1) {
      if (t2 == 1) {
        // printf(2, "impossible edge\n\r");
        switch_path_len = 0;
        return -1;
      } else {
        time = time - t2;
      }
    } else {
      time = time - t1;
    }

    path[len] = trackid;
    return find_path_by_sensor(train, speed, trackid, dest, 
                               track[trackid].edge[DIR_AHEAD].dest->index,
                               path, len + 1, time, total_time);

  } else if (this->type == NODE_BRANCH) {
    path[len] = trackid;

    int temp_len = switch_path_len;

    int straight_time = find_path_by_sensor(train, speed, origin, dest, 
                                       track[trackid].edge[DIR_STRAIGHT].dest->index, 
                                       path, len + 1, time, total_time);

    int temp_len_straight = switch_path_len;
    int size_straight = temp_len_straight - len - 1; 
    int temp_path[100];

    // if (path_found == 1) {
    //   return straight_time;
    // } 

    int i;
    for (i = 0; i < 100; i++) {
      temp_path[i] = switch_path[i];
    }

    switch_path_len = temp_len;
    int curved_time = find_path_by_sensor(train, speed, origin, dest, 
                                     track[trackid].edge[DIR_CURVED].dest->index, 
                                     path, len + 1, time, total_time);

    int temp_len_curved = switch_path_len;
    int size_curved = switch_path_len - len - 1;

    // printf(2, "branch, straight time: %d len: %d temp: %d size: %d, curved time: %d len: %d temp: %d size: %d\n\r", 
    //   straight_time, temp_len_straight, temp_len_straight, size_straight, curved_time, switch_path_len, temp_len_curved, size_curved);


    if (temp_len_straight == 0 && temp_len_curved == 0) {
      switch_path_len = 0;
      return -1;
    } else if (temp_len_straight == 0) {
      return curved_time;
    } else if (temp_len_curved == 0) {
      for (i = 0; i < 100; i++) {
        switch_path[i] = temp_path[i];
      }
      switch_path_time_diff = straight_time;
      switch_path_len = temp_len_straight;
      return straight_time;
    } else {
      int straight_abs = straight_time;
      int curved_abs = curved_time;
      if (straight_time < 0) straight_abs = -straight_time;
      if (curved_time < 0) curved_abs = -curved_time;

      if (straight_abs < curved_time) {
        for (i = 0; i < 100; i++) {
          switch_path[i] = temp_path[i];
        }
        switch_path_time_diff = straight_time;
        switch_path_len = temp_len_straight;
        return straight_time;
      } else {
        return curved_time;
      }
    }
    
  } else {
    // printf(2, "else trackid: %s, origin: %s, dest: %s\n\r", 
    //   track[trackid].name, track[origin].name, track[dest].name);
    path[len] = trackid;
    return find_path_by_sensor(train, speed, origin, dest, 
                               track[trackid].edge[DIR_AHEAD].dest->index, 
                               path, len + 1, time, total_time);
  } 
}

int update_train_state(int sensor) {
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
      track_node * next = get_next_sensor_or_branch(queue[queue_head]);
      // printf(2, "%s\n\r", next->name);
      if (next->type == NODE_SENSOR) {
        queue[queue_tail] = next->index;
        level_queue[queue_tail] = level_queue[queue_head] + 1;
        queue_tail += 1;
      } else if (next->type == NODE_BRANCH) {
        int next_after_branch = track[next->index].edge[DIR_STRAIGHT].dest->index;
        if (next_after_branch < 80) {
          queue[queue_tail] = track[next->index].edge[DIR_STRAIGHT].dest->index;
          level_queue[queue_tail] = level_queue[queue_head] + 1;
          queue_tail += 1;          
        }
        next_after_branch = track[next->index].edge[DIR_CURVED].dest->index;
        if (next_after_branch < 80) {
          queue[queue_tail] = track[next->index].edge[DIR_CURVED].dest->index;
          level_queue[queue_tail] = level_queue[queue_head] + 1;
          queue_tail += 1;          
        }
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
    // printf(2, "\033[14;40H\033[KPredicted Sensors: %d ", queue_head);
    // for (i = 0; i < queue_head; ++i) {
    //   printf(2, "%d ", queue[i]);
    // }
    printf(2, "\033[u");
    train_64_struct.missed_count = 0;
    if (train_64_path.in_progress) {
      train_64_path.err = 1;
      for (i = 0; i < train_64_path.len; ++i) {
        if (train_64_struct.cur_sensor == train_64_path.node[i]) {
          train_64_path.err = 0;
          // printf(2, "asdf");
          break;
        }
      }
    }
    if (train_64_path.err) {
      volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
      cancel_stop(64);
      if (td[INPUT_TID].state == PATH_SWITCH_BLOCKED) td[INPUT_TID].state = READY;
      train_64_path.in_progress = 0;
      printf(2, "\033[s\033[9;40H\033[K\033[0;31mFind path ended\033[0m\033[u");

    } else {
      if (train_64_path.in_progress) {
        if (train_64_path.len == -1) {
          printf(2, "\033[s\033[9;40H\033[K\033[0;31mNO PATH\033[0m\033[u");
        } else if (train_64_path.len >= 2) {
          int flag = 0;
          printf(2, "\033[s\033[9;40H\033[K\033[0;31m");
          for (i = 0; i < train_64_path.len; ++i) {
            if (track[train_64_path.node[i]].type == NODE_SENSOR) {
              if (flag) {
                printf(2, "\033[32m");
              }
              if (train_64_path.node[i] == last_sensor) {
                flag = 1;
              }
              printf(2, "%s ", track[train_64_path.node[i]].name);
            }
          }
          // if (flag) path_len = 0;
          printf(2, "\033[0m\033[u");
        }
      }
    }
    return 0;
  } else {
    train_64_struct.missed_count += 1;
    if (train_64_struct.missed_count == 3) {
      train_64_struct.cur_sensor = -1;
    }
    return 1;
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


int sensor_is_valid(int train, int cur_sensor, int time) {

  int prev_sensor = -1;
  int speed = -1; 

  if (train == 64) {
    prev_sensor = train_64_struct.prev_sensor;
    speed = train_64_struct.speed;
  }

  if (prev_sensor < 0 || prev_sensor > 80) {
    return -1;
  }

  track_node * next = get_next_sensor(prev_sensor);
  if (next->index == cur_sensor) {
    return 0;
  }

  int i;
  for (i = 0; i < 100; i++) {
    switch_path[i] = -1;
  }

  switch_path_time_diff = time;
  switch_path_len = 0;
  switch_time_index = 0;
  path_found = 0;

  volatile track_node * track = (track_node *) 0x01700000;

  // printf(2, "before findpath - tr: %d sp: %d p_sensor: %s c_sensor: %s time: %d\n\r", 
      // train, speed, track[prev_sensor].name, track[cur_sensor].name, time);
  int t = find_path_by_sensor(train, speed, prev_sensor, cur_sensor, prev_sensor, 
                                        switch_path, 0, time, 0);  

  // printf(2, "len: %d path: ", switch_path_len);
  // for (i = 0; i < switch_path_len; i++) {
  //   if (switch_path[i] == -1) {
  //     break;
  //   }
  //   printf(2, "%s ", track[switch_path[i]].name);
  // }
  // printf(2, "\n\r");

  return switch_path_len;
}

// this works as long as switch_path is valid
int check_missed_switch(int len, int cur_sensor, int train) {

  volatile track_node * track = (track_node *) 0x01700000;

  int origin = -1;

  if (train == 64) {
    origin = train_64_struct.prev_sensor;
  }

  int i = 0;
  while (i < switch_path_len) {
    // printf(2, "i: %d ", i);
    if (switch_path[i] == -1) break;
    if (track[switch_path[i]].index == track[origin].index) {
      i++;
      continue;
    }

    while(track[switch_path[i]].type != NODE_SENSOR) {
      if (track[switch_path[i]].type == NODE_BRANCH) {
        int dir;
        int this = switch_path[i];
        int next = switch_path[i + 1];
        if (track[this].edge[DIR_STRAIGHT].dest->index == track[next].index) {
          dir = DIR_STRAIGHT;
        } else {
          dir = DIR_CURVED;
        }
        
        if (track[this].dir != dir) {
          track[this].dir = dir;
          printf(2, "\033[s\033[11;40H\033[K\033[0;31mWrong Switch! %s\033[0m\033[u",
             track[this].name);
          printf(2, "\033[0;35m");
          update_switch(track[this].num, 33 + dir);
          printf(2, "\033[0m");
        }
      }
      i++;
    }

    if (track[switch_path[i]].type == NODE_SENSOR) {
      if (track[switch_path[i]].index != track[origin].index && 
          track[switch_path[i]].index != track[cur_sensor].index) {
        //s2 = i; 
        // missed sensor;
        printf(2, "\033[s\033[15;40H\033[K\033[0;31mMissed sensor: %s\033[0m\033[u",
           track[switch_path[i]].name);
      }
    }
    i++;    
  }
  return 0;
}

