#include "path_finding.h"
#include "track.h"
#include "user_syscall.h"
#include "courier.h"
#include "stop.h"
#include "clockserver.h"
#include "io.h"
#include "td.h"

#define MAX_PATH_LENGTH 100

#define train_76 0
#define train_71 1
#define train_64 2

short train_velocity[5][15][80][80] = {};
double default_speed[5][15] = {};

// int get_vol_ticks(int train_number, int speed, int sensor) {
//   // return vol_map[train_number][speed][sensor];
//   return 76.0;
// }

double get_velocity(int train_index, int speed, int now, int end, int dist) {
  if (train_velocity[train_index][speed][now][end] == 1) {
    return default_speed[train_index][speed];
  } else {
    return 1.0 * dist / train_velocity[train_index][speed][now][end];
  }
}

int target_sensor = 0;

int train_number_to_index(int x) {
  if(x == 76) return train_76; 
  if(x == 71) return train_71;
  if(x == 64) return train_64;
  return -1;
}

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

    double acc = 0.01745;
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
          int end = now + 1;
          int dist_sensor_to_sensor = track[path[now]].edge[track[path[now]].dir].dist;
          while (1 + 2 == 3) { // maybe include exit?
            if (track[path[end]].type == NODE_SENSOR) {
              break;
            } else {
              int temp = 0;
              if (track[path[end]].edge[DIR_STRAIGHT].dest->index == path[end + 1]) {
                track[path[end]].edge[DIR_STRAIGHT].dist;
              } else {
                track[path[end]].edge[DIR_CURVED].dist;
              }
              dist_sensor_to_sensor += temp;
              end += 1;
            }
          }
          int train_index = train_number_to_index(train_number);
          int speed = train_list_ptr[train_number];
          

          double v_0 = get_velocity(train_index, speed, path[now], path[end], dist_sensor_to_sensor);

          printf(2, "starting %s, ending %s, v: %d d: %d\n\r", track[path[now]].name, track[path[end]].name, (int)(v_0 * 100), dist_sensor_to_sensor);
          double temp = v_0 * v_0 / 2 / acc;
          printf(2, "temp %d, dist %d\n\r", (int)temp, (int)dist);
          if (temp <= dist) {
            double d_stop = dist - temp;
            double t_stop = d_stop / v_0;
            Stop(train_number, path[now], t_stop);
            printf(2, "\n\rcall stop at %s %d %d\n\r", track[path[now]].name, (int)(t_stop), (int) (v_0 * 100));
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


void train_velocity_init() {
  int i;
  short * const temp = (short *)train_velocity;
  for (i = 0; i < 5 * 15 * 80 * 80; ++i) {
    temp[i] = 1;
  }
  default_speed[train_64][10] = 4.958;

  train_velocity[train_64][10][2][42] = 73;
  train_velocity[train_64][10][2][44] = 115;
  train_velocity[train_64][14][3][31] = 68;
  train_velocity[train_64][10][3][31] = 88;
  train_velocity[train_64][10][16][61] = 80;
  train_velocity[train_64][14][16][61] = 60;
  train_velocity[train_64][10][29][63] = 41;
  train_velocity[train_64][10][30][2] = 85;
  train_velocity[train_64][10][31][41] = 72;
  train_velocity[train_64][14][31][41] = 58;
  train_velocity[train_64][10][31][36] = 95;
  train_velocity[train_64][10][17][40] = 69;
  train_velocity[train_64][10][20][50] = 80;
  train_velocity[train_64][14][21][43] = 60;
  train_velocity[train_64][10][21][43] = 72;
  train_velocity[train_64][10][41][16] = 73;
  train_velocity[train_64][14][41][16] = 60;
  train_velocity[train_64][10][42][20] = 66;
  train_velocity[train_64][10][43][3] = 72;
  train_velocity[train_64][14][43][3] = 57;
  train_velocity[train_64][10][44][70] = 175;
  train_velocity[train_64][10][45][3] = 120;
  train_velocity[train_64][10][46][59] = 83;
  train_velocity[train_64][10][47][37] = 53;
  train_velocity[train_64][10][36][46] = 60;
  train_velocity[train_64][10][36][74] = 206;
  train_velocity[train_64][10][37][30] = 94;
  train_velocity[train_64][10][40][30] = 75;
  train_velocity[train_64][10][48][29] = 91;
  train_velocity[train_64][10][57][52] = 140;
  train_velocity[train_64][10][57][55] = 160;
  train_velocity[train_64][10][58][47] = 80;
  train_velocity[train_64][10][59][74] = 52;
  train_velocity[train_64][10][60][17] = 80;
  train_velocity[train_64][10][61][77] = 56;
  train_velocity[train_64][14][61][77] = 41;
  train_velocity[train_64][10][63][77] = 60;
  train_velocity[train_64][10][50][68] = 55;
  train_velocity[train_64][10][51][21] = 81;
  train_velocity[train_64][14][51][21] = 60;
  train_velocity[train_64][14][52][69] = 57;
  train_velocity[train_64][10][52][69] = 73;
  train_velocity[train_64][10][53][56] = 137;
  train_velocity[train_64][10][53][73] = 125;
  train_velocity[train_64][10][54][56] = 153;
  train_velocity[train_64][10][55][71] = 80;
  train_velocity[train_64][10][56][75] = 73;
  train_velocity[train_64][10][73][76] = 71;
  train_velocity[train_64][10][74][57] = 71;
  train_velocity[train_64][10][75][37] = 200;
  train_velocity[train_64][10][75][58] = 51;
  train_velocity[train_64][10][76][60] = 55;
  train_velocity[train_64][14][77][72] = 60;
  train_velocity[train_64][10][77][72] = 73;
  train_velocity[train_64][10][66][48] = 33;
  train_velocity[train_64][10][68][53] = 107;
  train_velocity[train_64][10][69][51] = 106;
  train_velocity[train_64][14][69][51] = 90;
  train_velocity[train_64][10][69][66] = 57;
  train_velocity[train_64][10][70][54] = 124;
  train_velocity[train_64][10][71][45] = 228;
  train_velocity[train_64][14][72][52] = 104;
  train_velocity[train_64][10][72][52] = 123;
}

