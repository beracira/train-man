#include "path_finding.h"
#include "track.h"
#include "user_syscall.h"
#include "courier.h"
#include "stop.h"
#include "clockserver.h"
#include "io.h"
#include "td.h"

#define LEARNING_FACTOR 0.8

#define ORANGE 0
#define RED    1
#define BLUE   2
#define GREEN  3
#define CYAN   4
#define PINK   5


struct Train train_64_struct = {};
struct Train officer_struct = {};

int train_velocity[5][15][80][80] = {};
double default_speed[5][15] = {};

struct Path train_64_path = {};

int path[MAX_PATH_LENGTH] = {};
int path_len = 0;
int path_err = 0;

int train_acc[5][15][10] = {};
int sensor_type[80] = {};

// int get_vol_ticks(int train_number, int speed, int sensor) {
//   // return vol_map[train_number][speed][sensor];
//   return 76.0;
// }

// worker for path finding
// int path_finding_train_list[100] = {};
// int path_finding_dest_list[100] = {};
// volatile int path_finding_list_head = 0;
// volatile int path_finding_list_tail = 0;

// worker for path finding
// int PATH_TID = 0;

// worker for path finding
// void path_finding_init() {
//   path_finding_list_head = 0;
//   path_finding_list_tail = 0;  
//   int i;
//   for(i = 0; i < 100; ++i) {
//     path_finding_train_list[i] = 0;
//     path_finding_dest_list[i] = 0;
//   }
// }

// worker for path finding
// void path_finding_worker() {
//   path_finding_init();

//   PATH_TID = MyTid();

//   volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
//   volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;

//   while (1 * 1 == 1) {
//     if (path_finding_list_head == path_finding_list_tail) {
//       td[ks->tid].state = WORKER_BLOCKED;
//       Pass();
//     }
//     if (path_finding_list_head != path_finding_list_tail) {
//       int head = path_finding_list_head;
//       find_path(int train_number, int origin, int dest, int dist_init);      
//     }
//   }
// }

double get_velocity(int train_index, int speed, int now, int end, int dist) {
  printf(2, "\033[s\033[15;40H\033[Kti: %d sp: %d now: %d end: %d dist: %d tv: %d\033[u",
   train_index, speed, now, end, dist, train_velocity[train_index][speed][now][end]);
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
  if(x == 63) return train_63;
  if(x == 58) return train_58;
  return 0; // use the default spot
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

int find_path_dfs(int origin, int dest, int * path, int * acc_dist, int len) {
  volatile track_node * track = (track_node *) TRACK_ADDR;
  if (origin == dest) {
    int i;
    printf(2, "\033[s\033[10;40H\033[K\033[0;31m");
    for (i = 0; i < len; ++i) {
      if (track[path[i]].type == NODE_SENSOR)
        printf(2, "%s ", track[path[i]].name);
    }
    printf(2, "\033[9;40H\033[K\033[0m");
    int dist = 0;
    for (i = 0; i < len; ++i) {
      if (track[path[i]].type == NODE_SENSOR) {
        printf(2, "%d  ", dist);
      }
      if (track[path[i]].type == NODE_BRANCH) {
        if (track[path[i]].edge[DIR_STRAIGHT].dest->index == path[i + 1]) {
          dist += track[path[i]].edge[DIR_STRAIGHT].dist;
        } else {
          dist += track[path[i]].edge[DIR_CURVED].dist;
        }
      } else {
        dist += track[path[i]].edge[DIR_AHEAD].dist;
      }
    }
    printf(2, "\033[0m\033[u");
    return len;
  }
  if (exist(origin, path, len)) return 0;
  int type = track[origin].type;
  if (type == NODE_SENSOR || type == NODE_MERGE || type == NODE_ENTER) {
    path[len] = track[origin].edge[DIR_AHEAD].dest->index;
    return find_path_dfs(track[origin].edge[DIR_AHEAD].dest->index, dest, path, acc_dist, len + 1);
  } else if (type == NODE_BRANCH) {
    path[len] = track[origin].edge[DIR_STRAIGHT].dest->index;
    int retval = -1;
    retval = find_path_dfs(track[origin].edge[DIR_STRAIGHT].dest->index, dest, path, acc_dist, len + 1);
    if (retval > 0) return retval;
    path[len] = track[origin].edge[DIR_CURVED].dest->index;
    retval = find_path_dfs(track[origin].edge[DIR_CURVED].dest->index, dest, path, acc_dist, len + 1);
    if (retval > 0) return retval;
    return 0;
  } else {
    // exit
    return 0;
  }
}

int find_path_bfs(int origin, int dest, int * path, int * acc_dist, int len) {
  volatile track_node * track = (track_node *) TRACK_ADDR;

  char visited[200];
  int i;
  for (i = 0; i < 200; ++i) {
    visited[i] = 0;
  }
  int queue[1000];
  int queue_pre[1000];
  int head = 0;
  int tail = 1;
  queue[0] = origin & 0xfffffffe;
  queue[1] = (origin & 0xfffffffe) + 1;
  tail = 2;
  (void) len;
  (void) acc_dist;

  while (head != tail) {
    int dest_round_down = dest & 0xfffffffe;
    if (queue[head] == dest_round_down || queue[head] == dest_round_down + 1) {
      int len = 1;
      int cur = head;
      // int temp = 0;
      // while (temp <= head) {
      //   printf(2, "%s ", track[queue[temp]].name);
      //   temp += 1;
      // }
      while (cur != 0 && cur != 1) {
        cur = queue_pre[cur];
        len += 1;
      }
      int i;
      cur = head;
      for (i = len - 1; i >= 0; --i) {
        path[i] = queue[cur];
        cur = queue_pre[cur];
      }
      printf(2, "\033[s\033[9;40H\033[K\033[0;31m");
      for (i = 0; i < len; ++i) {
        if (track[path[i]].type == NODE_SENSOR)
          printf(2, "%s ", track[path[i]].name);
      }
      printf(2, "\033[0m\033[u");
      return len;
    }

    int type = track[queue[head]].type;
    int next_node;
    if (type == NODE_SENSOR || type == NODE_MERGE || type == NODE_ENTER) {
      next_node = track[queue[head]].edge[DIR_AHEAD].dest->index;
      if (visited[next_node] == 0) {
        queue[tail] = next_node;
        queue_pre[tail] = head;
        tail += 1;
        visited[next_node] = 1;        
      }
    } else if (type == NODE_BRANCH) {
      next_node = track[queue[head]].edge[DIR_STRAIGHT].dest->index;
      if (visited[next_node] == 0) {
        queue[tail] = next_node;
        queue_pre[tail] = head;
        tail += 1;        
        visited[next_node] = 1;
      }
      next_node = track[queue[head]].edge[DIR_CURVED].dest->index;
      if (visited[next_node] == 0) {
        queue[tail] = next_node;
        queue_pre[tail] = head;
        tail += 1;
        visited[next_node] = 1;
      }
    } else {
      // exit
    }
    head += 1;
  }
  return 0;
}

int find_path(int train_number, int origin, int dest, int dist_init) {
  volatile track_node * track = (track_node *) TRACK_ADDR;
  counter = 1;
  if (origin == dest) {
    // do something
    return 0;
  } else {
    struct Path * path = &train_64_path;
    int i;
    path->len = -1; // prevent get sensor data from print shit
    for (i = 0; i < MAX_PATH_LENGTH; ++i) path->node[i] = -1;
    path->node[0] = origin;
    path->len = find_path_dfs(origin, dest, path->node, path->predicted_arrival_time, 1);
    path->len = find_path_bfs(origin, dest, path->node, path->predicted_arrival_time, 1);
    // path_err = 0;
    int len = path->len;
    path->in_progress = 1;
    path->err = 0;

    int dist = dist_init;
    if (len >= 2) {
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
            path->in_progress = 0;
            return -1;
          }
        }
      }
    } else {
      path->len = -1;
      path->in_progress = 0;
      return -1;
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
    return 0;
  }
}

void update_train_velocity(int train_number, int speed, int start, int end, int new_time) {
  if (new_time >= 2000 || new_time <= 1) return;
  int train_index = train_number_to_index(train_number);
  if (train_velocity[train_index][speed][start][end] == 1) {
    train_velocity[train_index][speed][start][end] = new_time;
  } else {
    int old_time = train_velocity[train_index][speed][start][end];
    train_velocity[train_index][speed][start][end] = 
        ((LEARNING_FACTOR) * old_time + (1 - LEARNING_FACTOR) * new_time) + 0.5;
  }
}

void velocity_print(int train_number, int speed) {
  int train_index = train_number_to_index(train_number);
  int i, j;
  printf(2, "\033[s\n\r\033[K%d", train_index);
  for (i = 0; i < 80; ++i) {
    for (j = 0; j < 80; ++j) {
      if (train_velocity[train_index][speed][i][j] != 1) {
        printf(2, "%d %d %d; ", i, j, train_velocity[train_index][speed][i][j]);
      }
    }
  }
  printf(2, "\n\r\033[u");
}

void short_move(int train_number, int dist) {
  printf(2, "move %d %d\n\r", train_number, dist);
  if (train_number == 64) {
    float temp_dist = (float) (1.0 * dist / 10.0);
    float a = 8.1929;
    float b = -4.7666;
    float c = 1.6872 - temp_dist;

    // float a = 6.9729;
    // float b = -2.2272;
    // float c = 0.6139 - 40;

    float x = ((float)(-b) + fast_sqrt((float)(b * b - 4 * a * c))) / (float)(2.0 * a);

    // int ttt = (int) (b * b - 4 * a * c);
    // printf(2, "%d\n\r", ttt);
    // ttt = (int)(fast_sqrt(b * b - 4 * a * c) + 0.5);
    // printf(2, "%d\n\r", ttt);
    
    int temp = (int)(((x * (float)100) + 0.5) * 1.03);
    // printf(2, "%d\n\r", temp);

    if (temp < 30) temp = 30; 
    set_train_speed(train_number, 4);
    Delay(temp);
    set_train_speed(train_number, 0);    
  } else {
    // *&^%&#@$^#@$^%#$^&* dist in MM!!!
    float temp_dist = (float) (1.0 * dist / 10.0);
    float a = 7.63694;
    float b = -5.6789;
    float c = 2.4090909 - temp_dist;

    float x = ((float)(-b) + fast_sqrt((float)(b * b - 4 * a * c))) / (float)(2.0 * a);
    // int temp = fast_sqrt((float)(b * b - 4 * a * c)) * 100;
    // printf(2, "%d\n\r", temp);
    // temp = x;
    // printf(2, "%d\n\r", temp);
    

    int temp = (int)(((x * (float)100) + 0.5));
    // printf(2, "%d\n\r", temp);

    if (temp < 30) temp = 30; 
    set_train_speed(train_number, 4);
    Delay(temp);
    set_train_speed(train_number, 0);    

  }
}

void train_velocity_init() {

  path_len = -1;

  int i;
  int * const temp = (int *)train_velocity;
  for (i = 0; i < 5 * 15 * 80 * 80; ++i) {
    temp[i] = 1;
  }
  default_speed[train_64][6] = 3.3596;
  default_speed[train_64][8] = 3.50145;
  default_speed[train_64][10] = 4.958;
  default_speed[train_64][12] = 5.9843;
  default_speed[train_64][14] = 6.175;

  default_speed[train_63][6] = 3.3596;
  default_speed[train_63][8] = 3.50145;
  default_speed[train_63][10] = 4.958;
  default_speed[train_63][12] = 5.9843;
  default_speed[train_63][14] = 6.175;

  train_acc[train_63][10][ORANGE] = 770;
  train_acc[train_63][10][RED] = 770;
  train_acc[train_63][10][BLUE] = 770;
  train_acc[train_63][10][GREEN] = 753;
  train_acc[train_63][10][CYAN] = 686;
  train_acc[train_63][10][PINK] = 787;

  // train_acc[train_64][6][ORANGE] = 0.00832;
  // train_acc[train_64][6][RED] = 0.0094769;

  // train_acc[train_64][6][BLUE] = 0.00832;
  // train_acc[train_64][6][GREEN] = 0.00832;
  // train_acc[train_64][6][CYAN] = 0.0083;
  // train_acc[train_64][6][PINK] = 0.007755;

  train_acc[train_64][6][ORANGE] = 460;
  train_acc[train_64][6][RED] = 460;
  train_acc[train_64][6][BLUE] = 460;
  train_acc[train_64][6][GREEN] = 450;
  train_acc[train_64][6][CYAN] = 410;
  train_acc[train_64][6][PINK] = 470;

  train_acc[train_64][8][ORANGE] = 610;
  train_acc[train_64][8][RED] = 580;
  train_acc[train_64][8][BLUE] = 635;
  train_acc[train_64][8][GREEN] = 600;
  train_acc[train_64][8][CYAN] = 560;
  train_acc[train_64][8][PINK] = 600;

  train_acc[train_64][10][ORANGE] = 770;
  train_acc[train_64][10][RED] = 770;
  train_acc[train_64][10][BLUE] = 770;
  train_acc[train_64][10][GREEN] = 753;
  train_acc[train_64][10][CYAN] = 686;
  train_acc[train_64][10][PINK] = 787;

  train_acc[train_64][12][BLUE] = 995;
  train_acc[train_64][12][GREEN] = 975;
  train_acc[train_64][12][RED] = 930;
  train_acc[train_64][12][CYAN] = 1000;
  train_acc[train_64][12][PINK] = 1030;
  train_acc[train_64][12][ORANGE] = 1000;

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

  train_velocity[train_64][10][62][28] = 41;
  train_velocity[train_64][10][28][49] = 91;
  train_velocity[train_64][10][76][62] = 60;
  train_velocity[train_64][10][49][67] = 33;
  train_velocity[train_64][10][67][68] = 57;

  train_velocity[train_64][6][2][42] = 131;
  train_velocity[train_64][6][3][31] = 146;
  train_velocity[train_64][6][16][61] = 135;
  train_velocity[train_64][6][18][33] = 74;
  train_velocity[train_64][6][20][50] = 144;
  train_velocity[train_64][6][21][43] = 125;
  train_velocity[train_64][6][29][63] = 76;
  train_velocity[train_64][6][30][2] = 160;
  train_velocity[train_64][6][31][36] = 168;
  train_velocity[train_64][6][31][41] = 134;
  train_velocity[train_64][6][33][65] = 169;
  train_velocity[train_64][6][36][74] = 360;
  train_velocity[train_64][6][37][30] = 169;
  train_velocity[train_64][6][41][16] = 134;
  train_velocity[train_64][6][41][18] = 129;
  train_velocity[train_64][6][42][20] = 123;
  train_velocity[train_64][6][43][3] = 140;
  train_velocity[train_64][6][45][3] = 210;
  train_velocity[train_64][6][47][37] = 104;
  train_velocity[train_64][6][48][29] = 171;
  train_velocity[train_64][6][50][68] = 100;
  train_velocity[train_64][6][51][21] = 148;
  train_velocity[train_64][6][52][69] = 130;
  train_velocity[train_64][6][53][56] = 244;
  train_velocity[train_64][6][55][71] = 140;
  train_velocity[train_64][6][56][75] = 134;
  train_velocity[train_64][6][57][55] = 277;
  train_velocity[train_64][6][58][47] = 144;
  train_velocity[train_64][6][61][77] = 105;
  train_velocity[train_64][6][63][77] = 100;
  train_velocity[train_64][6][65][78] = 176;
  train_velocity[train_64][6][66][48] = 60;
  train_velocity[train_64][6][68][53] = 136;
  train_velocity[train_64][6][69][51] = 158;
  train_velocity[train_64][6][69][66] = 196;
  train_velocity[train_64][6][70][54] = 507;
  train_velocity[train_64][6][71][45] = 307;
  train_velocity[train_64][6][72][52] = 220;
  train_velocity[train_64][6][74][57] = 130;
  train_velocity[train_64][6][75][58] = 94;
  train_velocity[train_64][6][77][72] = 132;
  train_velocity[train_64][6][78][43] = 124;

  train_velocity[train_64][8][17][40] = 90;
  train_velocity[train_64][8][2][42] = 95;
  train_velocity[train_64][8][2][44] = 149;
  train_velocity[train_64][8][20][50] = 102;
  train_velocity[train_64][8][30][2] = 110;
  train_velocity[train_64][8][37][30] = 125;
  train_velocity[train_64][8][40][30] = 98;
  train_velocity[train_64][8][42][20] = 90;
  train_velocity[train_64][8][44][70] = 226;
  train_velocity[train_64][8][50][68] = 75;
  train_velocity[train_64][8][53][73] = 158;
  train_velocity[train_64][8][54][56] = 198;
  train_velocity[train_64][8][56][75] = 98;
  train_velocity[train_64][8][60][17] = 102;
  train_velocity[train_64][8][68][53] = 95;
  train_velocity[train_64][8][70][54] = 98;
  train_velocity[train_64][8][73][76] = 94;
  train_velocity[train_64][8][75][37] = 253;
  train_velocity[train_64][8][76][60] = 74;

  train_velocity[train_64][12][16][61] = 66;
  train_velocity[train_64][12][18][33] = 37;
  train_velocity[train_64][12][21][43] = 56;
  train_velocity[train_64][12][3][31] = 70;
  train_velocity[train_64][12][31][36] = 80;
  train_velocity[train_64][12][31][41] = 61;
  train_velocity[train_64][12][33][65] = 80;
  train_velocity[train_64][12][36][74] = 166;
  train_velocity[train_64][12][41][16] = 60;
  train_velocity[train_64][12][41][18] = 60;
  train_velocity[train_64][12][43][3] = 64;
  train_velocity[train_64][12][45][3] = 95;
  train_velocity[train_64][12][51][21] = 65;
  train_velocity[train_64][12][52][69] = 72;
  train_velocity[train_64][12][55][71] = 65;
  train_velocity[train_64][12][57][55] = 127;
  train_velocity[train_64][12][61][77] = 50;
  train_velocity[train_64][12][65][78] = 35;
  train_velocity[train_64][12][69][51] = 52;
  train_velocity[train_64][12][71][45] = 141;
  train_velocity[train_64][12][72][52] = 111;
  train_velocity[train_64][12][74][57] = 60;
  train_velocity[train_64][12][77][72] = 60;
  train_velocity[train_64][12][78][43] = 60;

}


int get_sensor_color(int sensor) {
  if (sensor == 2 || sensor == 3 || sensor == 30 || sensor == 31 || sensor == 36 || sensor == 37 || sensor == 74 || sensor == 75 || sensor == 56 || sensor == 57 || sensor == 54 || sensor == 55)
    return RED;

  if (sensor == 42 || sensor == 43 || sensor == 78 || sensor == 79 || sensor == 66 || sensor == 67 || sensor == 68 || sensor == 69 || sensor == 52 || sensor == 53 || sensor == 72 || sensor == 73 || sensor == 76 || sensor == 77 || sensor == 62 || sensor == 63 || sensor == 18 || sensor == 19 || sensor == 40 || sensor == 41)
    return BLUE;

  if (sensor == 20 || sensor == 21 || sensor == 50 || sensor == 51 || sensor == 70 || sensor == 71 || sensor == 16 || sensor == 17 || sensor == 60 || sensor == 61 || sensor == 46 || sensor == 47 || sensor == 58 || sensor == 59 || sensor == 38 || sensor == 39 || sensor == 34 || sensor == 35)
    return GREEN;

  if (sensor == 64 || sensor == 65 || sensor == 48 || sensor == 49 || sensor == 32 || sensor == 33 || sensor == 76 || sensor == 77)
    return CYAN;

  if (sensor == 44 || sensor == 45)
    return PINK;

  return ORANGE;
}
