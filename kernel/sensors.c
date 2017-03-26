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
#include "user_syscall.h"
#include "train_client.h"
#include "trackserver.h"
#include "sensor_table.h"

#define THE_EVIL_GUY 100
#define THE_OFFICER  200

#define SENSOR_ARRAY_SIZE 10
int RUNNING_TRAIN = 0;

int last_sensor = 0;
int sensor_requested = 0;

int switch_path[100] = {};
int switch_path_len = 0;

int time_to_next_sensor = 0;

int new_sensors[10] = {};
int new_sensors_len = 0;

int evil_sensor = 0;
int officer_sensor = 0;

int sensor_is_valid(int train, int cur_sensor, int time);
int check_missed_switch(int len, int cur_sensor, int train);
int sensor_attr();

int EVIL_TID = 0;
int OFFICER_TID = 0;

void get_sensor_data() {
  int sensors[10];
  int sensor_len = 0;
  int sensor_letter[SENSOR_ARRAY_SIZE];
  int sensor_digit[SENSOR_ARRAY_SIZE];
  int i;
  for (i = 0; i < SENSOR_ARRAY_SIZE; ++i) {
    sensor_letter[i] = 0;
    sensor_digit[i] = 0;
    new_sensors[i] = -1;
  }
  int tail = 0;
  sensor_requested = 0;

  evil_sensor = -1;
  officer_sensor = -1;

  //////////////////////////// 
  // velocity calibration

  // volatile track_node * track = (track_node *) 0x01700000;

 // int i = -1;
  // int d = 0;

  //////////////////////////// 

  while (1 + 2 == 3) {
    int c = Getc(1);
    sensors[sensor_len++] = c;

    if (sensor_len == 10) {
      int k;
      for (k = 0; k < SENSOR_ARRAY_SIZE; ++k) {
        new_sensors[k] = -1;
      }
      new_sensors_len = 0;
      for (k = 0; k < 5; ++k) {
        int high = sensors[k * 2];
        int low = sensors[k * 2 + 1];
        int radix = 16;
        while (low) {
          if (low & 1) {
            sensor_letter[tail] = 'A' + k;
            sensor_digit[tail] = radix;
            new_sensors[new_sensors_len++] = k * 16 + radix - 1;
            tail += 1;
            tail %= SENSOR_ARRAY_SIZE;
          }
          low >>= 1;
          radix--;
        }
        radix = 8;
        while (high) {
          if (high & 1) {
            sensor_letter[tail] = 'A' + k;
            sensor_digit[tail] = radix;
            new_sensors[new_sensors_len++] = k * 16 + radix - 1;
            tail += 1;
            tail %= SENSOR_ARRAY_SIZE;
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

      sensor_attr();

      print_sections();
      volatile track_node * track = (track_node *) 0x01700000;
      printf(2, "\033[s\033[20;40H\033[K evil: %s just: %s\033[u",
        track[evil_sensor].name, track[officer_sensor].name);
      printf(2, "\033[s\033[21;40H\033[K evil_cur: %s just_cur: %s\033[u",
        track[train_64_struct.cur_sensor].name, track[officer_struct.cur_sensor].name);
      printf(2, "\033[s\033[22;40H\033[K evil_cur: %d just_cur: %d\033[u",
        train_64_struct.speed, officer_struct.speed);

      // printf(2, "%d %d\n\r", evil_sensor, new_sensors_len);
      /* ---------     missed switches     ----------- */
      // check_missed_switch(last_sensor2, time);
      /* --------------------------------------------- */
    }
    sensor_requested = 0;
    volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
    if (td[CR_TID].state == SENSOR_BLOCKED) td[CR_TID].state = READY;
    if (td[EVIL_TID].state == GET_NEW_SENSOR_BLOCKED) td[EVIL_TID].state = READY;
    if (td[OFFICER_TID].state == GET_NEW_SENSOR_BLOCKED) td[OFFICER_TID].state = READY;
    if (target_sensor == last_sensor && td[INPUT_TID].state == PATH_SWITCH_BLOCKED) td[INPUT_TID].state = READY;
  }
}

void the_evil_guy() {
  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  volatile track_node * track = (track_node *) 0x01700000;
  int RUNNING_EVIL = train_64_struct.train_number;
  EVIL_TID = ks->tid;

  int prev_sensor = train_64_struct.cur_sensor;
  int prev_time = 0;
  int time = 0;

  while (3 * 3 == 9) {
    td[ks->tid].state = GET_NEW_SENSOR_BLOCKED;
    Pass();
    if (train_list_ptr[RUNNING_EVIL] == 0) {
      prev_time = time_ticks;
      continue;
    }

    if (evil_sensor == -1) continue;

    if (prev_sensor == -1) {
      if (evil_sensor != -1) {
        prev_sensor = evil_sensor;
        train_64_struct.prev_sensor = prev_sensor;
        reserve_section(track[evil_sensor].section, 0, RUNNING_EVIL);
      }
      prev_time = time_ticks;
      continue;
    } else if (prev_sensor == evil_sensor) {
      continue;
    }

    int result = update_train_state(THE_EVIL_GUY, evil_sensor);
    (void) result;

    train_64_struct.prev_sensor = evil_sensor;

    time = time_ticks - prev_time;
    prev_time = time_ticks;

    int predict_time = 0;
    int predict_dist = get_next_sensor_dist(prev_sensor);
    if (prev_sensor != -1) {
      predict_time = train_velocity[train_number_to_index(RUNNING_EVIL)][train_list_ptr[RUNNING_EVIL]][prev_sensor][evil_sensor];
      if (predict_time == 1) {
        int temp = default_speed[train_number_to_index(RUNNING_EVIL)][train_list_ptr[RUNNING_EVIL]];
        if (temp == -1 || temp == 0) predict_time = 0;
        else predict_time = predict_dist / default_speed[train_number_to_index(RUNNING_EVIL)][train_list_ptr[RUNNING_EVIL]];
      }
    }

    if (predict_time > 1200 || predict_time < 0) predict_time = 0;

    // printf(2, "\033[s\033[3;40H\033[K From %s to %s Predicted dist: %d\033[u",
    //   track[prev_sensor].name, track[evil_sensor].name, predict_dist);
    // printf(2, "\033[s\033[4;40H\033[K Predicted time: %d Actual time: %d Delta: %d Dist: %d\033[u", 
    //   predict_time, time, time - predict_time, (time / predict_dist) * (time - predict_time));
    // track_node * temp = get_next_sensor(evil_sensor);
    // printf(2, "\033[s\033[5;40H\033[K Next Sensor: %s\033[u", temp != 0 ? temp->name : "NULL");
    update_train_velocity(RUNNING_EVIL, train_list_ptr[RUNNING_EVIL], prev_sensor, evil_sensor, time);
    if (track[prev_sensor].section != track[evil_sensor].section) {
      reserve_section(track[evil_sensor].section, track[prev_sensor].section, RUNNING_EVIL);
    }
    // printf(2, "\033[s\033[19;40H\033[Kprev section: %d evil section: %d\033[u", track[prev_sensor].section, track[evil_sensor].section);
    // printf(2, "\033[s\033[20;40H\033[Kprev : %s evil : %s\033[u", track[prev_sensor].name, track[evil_sensor].name);
    // printf(2, "\033[s\033[21;40H\033[Ka : %s b : %d\033[u", track[20].name, track[20].section);
    // printf(2, "\033[s\033[22;40H\033[Kprev : %d evil : %d\033[u", prev_sensor, evil_sensor);
    // print_sections();
    prev_sensor = evil_sensor;
    if (td[EVIL_WORKER_TID].state == PATH_SENSOR_BLOCKED) td[EVIL_WORKER_TID].state = READY;
  }
}

void the_officer() {
  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  volatile track_node * track = (track_node *) 0x01700000;
  int RUNNING_JUST = officer_struct.train_number;
  OFFICER_TID = ks->tid;

  int prev_sensor = officer_struct.cur_sensor;
  int prev_time = 0;
  int time = 0;

  while (5 * 5 == 25) {
    td[ks->tid].state = GET_NEW_SENSOR_BLOCKED;
    Pass();
    if (train_list_ptr[RUNNING_JUST] == 0) {
      prev_time = time_ticks;
      continue;
    }

    if (officer_sensor == -1) continue;

    if (prev_sensor == -1) {
      if (officer_sensor != -1) {
        prev_sensor = officer_sensor;
        officer_struct.prev_sensor = prev_sensor;
        reserve_section(track[officer_sensor].section, 0, RUNNING_JUST);
      }
      prev_time = time_ticks;
      continue;
    } else if (prev_sensor == officer_sensor) {
      continue;
    }

    int result = update_train_state(THE_OFFICER, officer_sensor);
    (void) result;

    officer_struct.prev_sensor = officer_sensor;

    time = time_ticks - prev_time;
    prev_time = time_ticks;

    int predict_time = 0;
    int predict_dist = get_next_sensor_dist(prev_sensor);
    if (prev_sensor != -1) {
      predict_time = train_velocity[train_number_to_index(RUNNING_JUST)][train_list_ptr[RUNNING_JUST]][prev_sensor][officer_sensor];
      if (predict_time == 1) {
        int temp = default_speed[train_number_to_index(RUNNING_JUST)][train_list_ptr[RUNNING_JUST]];
        if (temp == -1 || temp == 0) predict_time = 0;
        else predict_time = predict_dist / default_speed[train_number_to_index(RUNNING_JUST)][train_list_ptr[RUNNING_JUST]];
      }
    }

    if (predict_time > 1200 || predict_time < 0) predict_time = 0;

    // printf(2, "\033[s\033[33;40H\033[K From %s to %s Predicted dist: %d\033[u",
    //   track[prev_sensor].name, track[officer_sensor].name, predict_dist);
    // printf(2, "\033[s\033[34;40H\033[K Predicted time: %d Actual time: %d Delta: %d Dist: %d\033[u", 
    //   predict_time, time, time - predict_time, (time / predict_dist) * (time - predict_time));
    // track_node * temp = get_next_sensor(officer_sensor);
    // printf(2, "\033[s\033[35;40H\033[K Next Sensor: %s\033[u", temp != 0 ? temp->name : "NULL");
    update_train_velocity(RUNNING_JUST, train_list_ptr[RUNNING_JUST], prev_sensor, officer_sensor, time);
    if (track[prev_sensor].section != track[officer_sensor].section) {
      reserve_section(track[officer_sensor].section, track[prev_sensor].section, RUNNING_JUST);
    }
    // printf(2, "\033[s\033[19;40H\033[Kprev section: %d evil section: %d\033[u", track[prev_sensor].section, track[officer_sensor].section);
    // printf(2, "\033[s\033[20;40H\033[Kprev : %s evil : %s\033[u", track[prev_sensor].name, track[officer_sensor].name);
    // printf(2, "\033[s\033[21;40H\033[Ka : %s b : %d\033[u", track[20].name, track[20].section);
    // printf(2, "\033[s\033[22;40H\033[Kprev : %d evil : %d\033[u", prev_sensor, officer_sensor);
    // print_sections();
    prev_sensor = officer_sensor;
    if (td[OFFICER_WORKER_TID].state == PATH_SENSOR_BLOCKED) td[OFFICER_WORKER_TID].state = READY;
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
  train_64_struct.stopping = 0;

  int i;
  for (i = 0; i < 100; ++i) {
    train_64_struct.predict_sensors[i] = 0;
  }

  officer_struct.cur_sensor = -1;
  officer_struct.direction = 1;
  officer_struct.missed_count = 0;
  officer_struct.prev_sensor = -1;
  officer_struct.speed = 0;
  officer_struct.time_current_sensor = -1;
  officer_struct.stopping = 0;

  for (i = 0; i < 100; ++i) {
    officer_struct.predict_sensors[i] = 0;
  }
}

int switch_time_index = 0;
int switch_path_time_diff = 0;
int path_found = 0;

int sensor_attr() {
  if (new_sensors_len == 0) {
    return 1;
  } else if (new_sensors_len == 1) {
    if (train_64_struct.speed != 0 || train_64_struct.stopping) {
      evil_sensor = new_sensors[0];
    }
    if (officer_struct.speed != 0 || officer_struct.stopping) {
      officer_sensor = new_sensors[0];
    }
  } else {
    if (train_64_struct.speed != 0 || train_64_struct.stopping) {
      int i;
      for (i = 0; i < SENSOR_ARRAY_SIZE; ++i) {
        if (new_sensors[i] != -1 && train_64_struct.predict_sensors[new_sensors[i]]) {
          evil_sensor = new_sensors[i];
          new_sensors[i] = -1;
          break;
        }
      }
    }
    if (officer_struct.speed != 0 || officer_struct.stopping) {
      int i;
      for (i = 0; i < SENSOR_ARRAY_SIZE; ++i) {
        if (new_sensors[i] != -1 && officer_struct.predict_sensors[new_sensors[i]]) {
          officer_sensor = new_sensors[i];
          new_sensors[i] = -1;
          break;
        }
      }
    }
  }
  return 0;
}

int find_valid_sensor(int client) {
  struct Train * client_ptr;
  if (client == THE_EVIL_GUY) {
    client_ptr = &train_64_struct;
  } else {
    client_ptr = &officer_struct;
  }

  if (client_ptr->cur_sensor == -1) {
    return new_sensors[0];
  }
  int i;
  for (i = 0; i < SENSOR_ARRAY_SIZE; ++i) {
    if (new_sensors[i] != -1 && client_ptr->predict_sensors[new_sensors[i]]) {
      return new_sensors[i];
    }
  }
  client_ptr->missed_count += 1;
  if (client_ptr->missed_count == 3) {
    client_ptr->cur_sensor = -1;
  }
  return -1;
}

int update_train_state(int client, int sensor) {
  volatile track_node * track = (track_node *) 0x01700000;
  struct Train * client_ptr = 0;
  if (client == THE_EVIL_GUY) {
    client_ptr = &train_64_struct;
  } else {
    client_ptr = &officer_struct;
  }
  // printf(2, "%d\n\r", train_64_struct.cur_sensor);
  if (client_ptr->predict_sensors[sensor] || client_ptr->cur_sensor == -1) {
    client_ptr->cur_sensor = sensor;
    predict_path(sensor);
    predict_sensors(client, sensor);
    client_ptr->missed_count = 0;
    // if (train_64_path.in_progress) {
    //   train_64_path.err = 1;
    //   for (i = 0; i < train_64_path.len; ++i) {
    //     if (client_ptr->cur_sensor == train_64_path.node[i]) {
    //       train_64_path.err = 0;
    //       // printf(2, "asdf");
    //       break;
    //     }
    //   }
    // }
    // if (train_64_path.err) {
    //   volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
    //   cancel_stop(RUNNING_TRAIN);
    //   if (td[INPUT_TID].state == PATH_SWITCH_BLOCKED) td[INPUT_TID].state = READY;
    //   train_64_path.in_progress = 0;
    //   printf(2, "\033[s\033[9;40H\033[K\033[0;31mFind path ended\033[0m\033[u");

    // } else {
    //   if (train_64_path.in_progress) {
    //     if (train_64_path.len == -1) {
    //       printf(2, "\033[s\033[9;40H\033[K\033[0;31mNO PATH\033[0m\033[u");
    //     } else if (train_64_path.len >= 2) {
    //       int flag = 0;
    //       printf(2, "\033[s\033[9;40H\033[K\033[0;31m");
    //       for (i = 0; i < train_64_path.len; ++i) {
    //         if (track[train_64_path.node[i]].type == NODE_SENSOR) {
    //           if (flag) {
    //             printf(2, "\033[32m");
    //           }
    //           if (train_64_path.node[i] == last_sensor) {
    //             flag = 1;
    //           }
    //           printf(2, "%s ", track[train_64_path.node[i]].name);
    //         }
    //       }
    //       // if (flag) path_len = 0;
    //       printf(2, "\033[0m\033[u");
    //     }
    //   }
    // }
    return 0;
  } else {
    client_ptr->missed_count += 1;
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

void predict_sensors(int client, int sensor) {
  volatile track_node * track = (track_node *) 0x01700000;
  struct Train * client_ptr = 0;
  if (client == THE_EVIL_GUY) {
    client_ptr = &train_64_struct;
  } else {
    client_ptr = &officer_struct;
  }

  int queue[100];
  int level_queue[100];
  int queue_head = 0;
  int queue_tail = 1;
  int i;
  for (i = 0; i < 100; ++i) {
    client_ptr->predict_sensors[i] = 0;
    queue[i] = 0;
    level_queue[i] = 0;
  }

  queue[0] = sensor;
  level_queue[0] = 0;
  while (queue_head != queue_tail) {
    // printf(2, "%s\n\r", track[queue[queue_head]].name);
    if (level_queue[queue_head] == 1) {
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
      // if (next_after_branch < 80) {
        queue[queue_tail] = track[next->index].edge[DIR_STRAIGHT].dest->index;
        level_queue[queue_tail] = level_queue[queue_head];
        if (next_after_branch < 80) {
          level_queue[queue_tail] += 1;
        }
        queue_tail += 1;          
      // }
      next_after_branch = track[next->index].edge[DIR_CURVED].dest->index;
      // if (next_after_branch < 80) {
        queue[queue_tail] = track[next->index].edge[DIR_CURVED].dest->index;
        level_queue[queue_tail] = level_queue[queue_head];
        if (next_after_branch < 80) {
          level_queue[queue_tail] += 1;
        }
        queue_tail += 1;          
      // }
    }
    queue_head += 1;
    continue;
  }
  for (i = 0; i < queue_head; ++i) {
    if (queue[i] < 80) {
      client_ptr->predict_sensors[queue[i]] = 1;
    }
  }
  printf(2, "\033[s\033[13;40H\033[KPredicted Sensors: %d ", queue_head);
  for (i = 0; i < 100; ++i) {
    if (client_ptr->predict_sensors[i] != 0) {
      printf(2, "%s ", track[i].name);
    }
  }
  int next_sensor = get_next_sensor(client_ptr->cur_sensor)->index;
  // int dist_to_next_sensor = get_next_sensor_dist(client_ptr->cur_sensor);
  time_to_next_sensor = 
  train_velocity[train_number_to_index(client_ptr->train_number)][client_ptr->speed][client_ptr->cur_sensor][next_sensor];
  time_to_next_sensor /= 10;
  // printf(2, "\033[14;40H\033[KTime to next sensor: %d ", queue_head);
  // for (i = 0; i < queue_head; ++i) {
  //   printf(2, "%d ", queue[i]);
  // }
  printf(2, "\033[u");

  // if (sensor == C7) client_ptr->predict_sensors[E11] = 1;

}

