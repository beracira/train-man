#include "train_client.h"
#include "path_finding.h"
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

int EVIL_WORKER_TID = 0;

void the_evil_worker() {
  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  volatile track_node * track = (track_node *) 0x01700000;
  EVIL_WORKER_TID = ks->tid;

  struct Train * this = &train_64_struct;
  struct Path * path = &train_64_path;

  while (1) {
    td[ks->tid].state = WORKER_BLOCKED;
    Pass();

    if (this->cur_sensor == this->dest) continue;

    int len = find_path_bfs(this->cur_sensor, this->dest, path->node, 0, 0);
    path->len = len;

    int reversed = 0;
    if (len < 2) {
      this->dest = -1;
      path->len = -1;
      path->in_progress = 0;
      continue;
    } else {
      int dist = 0;
      int now = len - 2;

      if (path->node[0] != train_64_struct.cur_sensor) {
        train_64_struct.cur_sensor = track[train_64_struct.cur_sensor].reverse->index;
        reversed = 1;
      }
      while (now >= 0) {
        if (track[path->node[now]].type != NODE_SENSOR) {
          if (track[path->node[now]].type == NODE_BRANCH) {
            if (path->node[now + 1] == track[path->node[now]].edge[DIR_STRAIGHT].dest->index) {
              dist += track[path->node[now]].edge[DIR_STRAIGHT].dist;
            } else {
              dist += track[path->node[now]].edge[DIR_CURVED].dist;
            }
          } else {
            dist += track[path->node[now]].edge[DIR_AHEAD].dist;
          }
          now -= 1;
        } else {
          // if (now == len - 1) // stop after a sensor. with long dist;
          dist += track[path->node[now]].edge[DIR_AHEAD].dist;
          // double v_0 = get_vol(train_number, train_list_ptr[train_number], path->node[now]);
          int end = now + 1;
          int dist_sensor_to_sensor = track[path->node[now]].edge[0].dist; //***
          while (track[path->node[end]].type != NODE_SENSOR) { // maybe include exit?
            int temp = 0;
            if (track[path->node[end]].edge[DIR_STRAIGHT].dest->index == path->node[end + 1]) {
              temp = track[path->node[end]].edge[DIR_STRAIGHT].dist;
            } else {
              temp = track[path->node[end]].edge[DIR_CURVED].dist;
            }
            dist_sensor_to_sensor += temp;
            end += 1;
          }
          int train_index = train_number_to_index(this->train_number);
          int speed = this->speed;
          if (speed == 0) speed = 10;

          double v_0 = get_velocity(train_index, speed, path->node[now], path->node[end], dist_sensor_to_sensor);

          // printf(2, "\n\rstarting %s, ending %s, v: %d d: %d\n\r", track[path->node[now]].name, track[path->node[end]].name, (int)(v_0 * 100), dist_sensor_to_sensor);
          // double acc = train_acc[train_index][speed][get_sensor_color(path->node[now])];
          int stopping_dist = train_acc[train_index][speed][get_sensor_color(path->node[now])];
          // double temp = v_0 * v_0 / 2 / acc;
          // printf(2, "temp %d, dist %d\n\r", (int)temp, (int)dist);

          if (now == 0) {
            int i;
            for (i = 0; i < len; ++i) {
              if (track[path->node[i]].type == NODE_BRANCH) {
                if (path->node[i + 1] != track[path->node[i]].edge[track[path->node[i]].dir].dest->index) {
                  int temp = 1 - track[path->node[i]].dir;
                  flip_switch(track[path->node[i]].num, 33 + temp);
                  Delay(10);
                  flip_switch(track[path->node[i]].num, 33 + temp);
                  Delay(10);
                }        
              }
            }

            if (reversed) {
              // train_64_struct.cur_sensor = track[train_64_struct.cur_sensor].reverse->index;
              set_train_speed(this->train_number, 15);
              Delay(100);
            }
            while (dist > 0) { // keep doing short move
              // printf(2, "\033[s\033[19;40H\033[Kbefore cur_sensor: %d", this->cur_sensor);
              short_move(train_64_struct.train_number, dist >= 240 ? 240 : dist);
              // printf(2, "\033[s\033[20;40H\033[Kafter cur_sensor: %d", this->cur_sensor);
              dist -= 240;
              Delay(200);
            }
            path->len = -1;
            // this->dest = -1;
            this->cur_sensor = this->dest;
            path->in_progress = 0;
            now = -2;
            break;
          } else if (stopping_dist <= dist) {
            double d_stop = dist - stopping_dist;
            int t_stop = d_stop / v_0;
            if (t_stop < 0) t_stop = 0;
            Stop(this->train_number, path->node[now], t_stop);
            printf(2, "\033[s\033[14;40H\033[Kcall stop at %s %d %d %d ss: %d dist: %d\033[u", track[path->node[now]].name, (int)(d_stop), (int)(t_stop), (int) (v_0 * 100), dist_sensor_to_sensor, dist);
            // printf(2, "\033[s\033[15;40H\033[ti: %d speed: %d %d ss: %d dist: %d\033[u", track[path->node[now]].name, (int)(d_stop), (int)(t_stop), (int) (v_0 * 100), dist_sensor_to_sensor, dist);
            break;
          } else if (stopping_dist > dist) {
            now -= 1;
          }
        }
      }
      // reached only when a short move is done.
      if (now == -2) {
        printf(2, "short move done\n\r");
        continue;
      }
    }


    if (reversed) {
      set_train_speed(this->train_number, 15);
      Delay(100);
    }
    set_train_speed(this->train_number, 10);
    target_sensor = -1;
    int target_sensor_closer = -1;

    int i;
    for (i = 0; i < len; ++i) {
      if (track[path->node[i]].type == NODE_BRANCH) {
        if (path->node[i + 1] != track[path->node[i]].edge[track[path->node[i]].dir].dest->index) {
          int temp = 1 - track[path->node[i]].dir;
          flip_switch(track[path->node[i]].num, 33 + temp);
          Delay(10);
          flip_switch(track[path->node[i]].num, 33 + temp);
          Delay(10);
        }        
        break;
      }    
    }

    for (i = 1; i < len; ++i) {

      if (track[path->node[i]].type == NODE_BRANCH) {
      } else if (track[path->node[i]].type == NODE_SENSOR) {
        target_sensor = target_sensor_closer;
        target_sensor_closer = path->node[i];
      }
    }

    i = 1;

    while ((2 & 1) == 0) {
      printf(2, "wait on: %s\n\r", track[path->node[i]].name);
      td[ks->tid].state = PATH_SENSOR_BLOCKED;
      Pass();
      printf(2, "get: %s\n\r", track[this->cur_sensor].name);
      if (this->cur_sensor == path->node[i]) {
        int j;
        int seen_sensor = 0;
        for (j = i + 1; j < len; ++j) {
          if (track[path->node[j]].type == NODE_SENSOR) {
            if (seen_sensor) {
              i = seen_sensor;
              break;
            } else {
              seen_sensor = j;
            }
          } else if (track[path->node[j]].type == NODE_BRANCH) {
            if (path->node[j + 1] != track[path->node[j]].edge[track[path->node[j]].dir].dest->index) {
              int temp = 1 - track[path->node[j]].dir;
              flip_switch(track[path->node[j]].num, 33 + temp);
              Delay(10);
              flip_switch(track[path->node[j]].num, 33 + temp);
              Delay(10);
            }
          }
        }
        if (j == len - 1) {
          printf(2, "goto on: %s\n\r", track[path->node[i]].name);
          i = j;
          this->cur_sensor = path->node[i];
          break;
        }
      }
    }
  }
}
