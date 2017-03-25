#include "train_client.h"
#include "path_finding.h"

int EVIL_WORKER_TID = 0;

void evil_worker() {
  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  EVIL_WORKER_TID = ks->tid;

  struct Train * this = &train_64_struct;
  struct Path * path = &train_64_path;

  while (1) {
    td[ks->tid] = WORKER_BLOCKED;
    Pass();

    if (this->cur_sensor == this->dest) continue;

    int len = find_path_bfs(this->origin, this->dest, this->node, 0, 0);
    path->len = len;
    if (len < 2) {
      this->dest = -1;
      path->len = -1;
      path->in_progress = 0;
    } else {
      int now = len - 2;
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
          int train_index = train_number_to_index(train_number);
          int speed = train_list_ptr[train_number];

          double v_0 = get_velocity(train_index, speed, path->node[now], path->node[end], dist_sensor_to_sensor);

          // printf(2, "\n\rstarting %s, ending %s, v: %d d: %d\n\r", track[path->node[now]].name, track[path->node[end]].name, (int)(v_0 * 100), dist_sensor_to_sensor);
          // double acc = train_acc[train_index][speed][get_sensor_color(path->node[now])];
          int stopping_dist = train_acc[train_index][speed][get_sensor_color(path->node[now])];
          // double temp = v_0 * v_0 / 2 / acc;
          // printf(2, "temp %d, dist %d\n\r", (int)temp, (int)dist);
          if (stopping_dist <= dist) {
            double d_stop = dist - stopping_dist;
            int t_stop = d_stop / v_0;
            if (t_stop < 0) t_stop = 0;
            Stop(train_number, path->node[now], t_stop);
            printf(2, "\033[s\033[13;40H\033[Kcall stop at %s %d %d %d ss: %d dist: %d\033[u", track[path->node[now]].name, (int)(d_stop), (int)(t_stop), (int) (v_0 * 100), dist_sensor_to_sensor, dist);
            break;
          } else if (stopping_dist > dist && now != 0) {
            now -= 1;
          } else {
            // cannot stop GG
            path->len = -1;
            this->dest = -1;
            path->in_progress = 0;
            continue;
          }
        }
      }
    }
    volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
    volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
    target_sensor = -1;
    int target_sensor_closer = -1;
    for (i = 0; i < len; ++i) {
      if (path->err) return 1;
      if (track[path->node[i]].type == NODE_BRANCH) {
        if (path->node[i + 1] != track[path->node[i]].edge[track[path->node[i]].dir].dest->index) {
          if (target_sensor == -1) {
            int temp = 1 - track[path->node[i]].dir;
            flip_switch(track[path->node[i]].num, 33 + temp);
            Delay(10);
            flip_switch(track[path->node[i]].num, 33 + temp);
            Delay(10);
          } else {
            // printf(2, "\n\rblocked");
            td[ks->tid].state = PATH_SWITCH_BLOCKED;
            Pass();
            // printf(2, "\n\runblocked");
            int temp = 1 - track[path->node[i]].dir;
            flip_switch(track[path->node[i]].num, 33 + temp);
            Delay(10);
            flip_switch(track[path->node[i]].num, 33 + temp);
            Delay(10);
          }
        }
      } else if (track[path->node[i]].type == NODE_SENSOR) {
        target_sensor = target_sensor_closer;
        target_sensor_closer = path->node[i];
      }
    }
  }
}
