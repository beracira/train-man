#include "path_finding.h"
#include "track.h"
#include "user_syscall.h"
#include "courier.h"
#include "stop.h"
#include "clockserver.h"
#include "io.h"
#include "td.h"

#define MAX_PATH_LENGTH 100



// int get_vol_ticks(int train_number, int speed, int sensor) {
//   // return vol_map[train_number][speed][sensor];
//   return 76.0;
// }

int target_sensor = 0;

int exist(int origin, int * path, int len) {
  int i;
  int origin_count = 0;
  for (i = 0; i < len; ++i) {
    if (path[i] == origin) ++origin_count;
  }
  return origin_count >= 2;
}

static int counter = 0;

int find_path_dfs(int origin, int dest, int * path, int len) {
  volatile track_node * track = (track_node *) TRACK_ADDR;
  if (origin == dest) {
    int i;
    printf(2, "\033[s\033[H");
    for (i = 0; i < len; ++i) {
      printf(2, "%s ", track[path[i]].name);
    }
    printf(2, "\033[u");
    return len;
  }
  if (exist(origin, path, len)) return 0;
  int type = track[origin].type;
  if (type == NODE_SENSOR || type == NODE_MERGE || type == NODE_ENTER) {
    path[len] = track[origin].edge[DIR_AHEAD].dest->index;
    return find_path_dfs(track[origin].edge[DIR_AHEAD].dest->index, dest, path, len + 1);
  } else if (type == NODE_BRANCH) {
    path[len] = track[origin].edge[DIR_STRAIGHT].dest->index;
    int retval = -1;
    retval = find_path_dfs(track[origin].edge[DIR_STRAIGHT].dest->index, dest, path, len + 1);
    if (retval > 0) return retval;
    path[len] = track[origin].edge[DIR_CURVED].dest->index;
    retval = find_path_dfs(track[origin].edge[DIR_CURVED].dest->index, dest, path, len + 1);
    if (retval > 0) return retval;
    return 0;
  } else {
    // exit
    return 0;
  }
}
int find_path(int train_number, int origin, int dest) {
  volatile track_node * track = (track_node *) TRACK_ADDR;
  counter = 1;
  if (origin == dest) {
    // do something
    return 0;
  } else {
    int path[MAX_PATH_LENGTH];
    int i;
    for (i = 0; i < MAX_PATH_LENGTH; ++i) path[i] = -1;
    path[0] = origin;
    int len = find_path_dfs(origin, dest, path, 1);

    double acc = 15.87302 / 10;
    int dist = 0;
    if (len >= 2) {
      int now = len - 2;
      while (now >= 0) {
        if (track[path[now]].type != NODE_SENSOR) {
          if (track[path[now]].type == NODE_BRANCH) {
            if (path[now + 1] == track[path[now]].edge[DIR_STRAIGHT].dest->index) {
              dist += track[path[now]].edge[DIR_STRAIGHT].dist;
            } else {
              dist += track[path[now]].edge[DIR_CURVED].dist;
            }
          } else {
            dist += track[path[now]].edge[DIR_AHEAD].dist;
          }
          now -= 1;
        } else {
          dist += track[path[now]].edge[DIR_AHEAD].dist;
          // double v_0 = get_vol(train_number, train_list_ptr[train_number], path[now]);
          double v_0 = 3.6092;
          double temp = v_0 * v_0 / 2 / acc;
          if (temp <= dist) {
            double d_stop = dist - temp;
            double t_stop = d_stop / v_0;
            Stop(train_number, path[now], t_stop);
            break;
          } else if (temp > dist && now != 0) {
            now -= 1;
          } else {
            // cannot stop GG
            return -1;
          }
        }
      }
    } else {
      return -1;
    }

    volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
    volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
    target_sensor = -1;
    int target_sensor_closer = -1;
    for (i = 0; i < len; ++i) {
      if (track[path[i]].type == NODE_BRANCH) {
        if (path[i + 1] != track[path[i]].edge[track[path[i]].dir].dest->index) {
          if (target_sensor == -1) {
            flip_switch(track[path[i]].num, 33 + (1 - track[path[i]].dir));
            Delay(20);            
          } else {
            printf(2, "\n\rblocked");
            td[ks->tid].state = PATH_SWITCH_BLOCKED;
            Pass();
            printf(2, "\n\runblocked");
            flip_switch(track[path[i]].num, 33 + (1 - track[path[i]].dir));
            Delay(20);            
          }
        }
      } else if (track[path[i]].type == NODE_SENSOR) {
        target_sensor = target_sensor_closer;
        target_sensor_closer = path[i];
      }
    }
    return 0;
  }
}

