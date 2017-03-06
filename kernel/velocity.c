#include "velocity.h"
#include "clockserver.h"
#include "user_syscall.h"
#include "track.h"
#include "common.h"

//  train_velocity[train num][speed][sensor id][dir]
//int train_velocity[MAX_TRAINS][MAX_SPEED][MAX_SENSORS][2];

// train data
// void init_train_velocity(int track) {
//   if (track == TRACK_A) {
//     // do fun things
//   } else {
//     // fun things pt 2
//   }
// }

int VEL_TID = 0;

void sensor_data_received(int i) {
  struct vel_request input;
  struct vel_request output;

  input.type = SENSOR_ACTIVATED;
  input.sensor = i;

  // double check tid
  Send(VEL_TID, &input, sizeof(struct vel_request), &output, sizeof(struct vel_request));

}

void velocity_server() {
  VEL_TID = MyTid();

  int sender_tid = -1;
  struct vel_request req;
  struct vel_request result;

  struct train_velocity tv[80];

  int prev_time = 0;

  int time = 0;

  int v = 0;

  int prev_sensor = -1;
  volatile track_node * track = (track_node *) 0x01700000;

  int i = -1;
  int d = -1;
  int y = 5;

  printf(2, "\033[s");
  for (i = 0; i < 80; i++) {
    tv[i].length = track[i].edge[DIR_AHEAD].dist;
    tv[i].num = 0;
    tv[i].time = 0;

    if (i < 20) {
      printf(2, "\033[%d;40H", i + y);
    } else if (i < 40){
      printf(2, "\033[%d;70H", i % 20 + y);      
    } else if (i < 60){
      printf(2, "\033[%d;100H", i % 20 + y);      
    } else {
      printf(2, "\033[%d;130H", i % 20 + y);      
    }
  
    printf(2, "%s:%d t:%d n:%d sw:%d", 
                  track[i].name, i, 
                  tv[i].time, tv[i].num, -1);
  }
  printf(2, "\033[u");

  while(1) {
    Receive( &sender_tid, &req, sizeof(struct vel_request));

    if (req.type == SENSOR_ACTIVATED){

        i = req.sensor;

        if (prev_sensor == -1) {
          prev_sensor = i;
          break;
        } else if (prev_sensor == i) {
          break;
        }

        if (track[prev_sensor].edge[DIR_AHEAD].dest->type == NODE_BRANCH){
          d = track[prev_sensor].edge[DIR_AHEAD].dest->dir;
        } else {
          d = -1;
        }

        time = time_ticks - prev_time;
        prev_time = time_ticks;

        tv[prev_sensor].time = ((tv[prev_sensor].time * tv[prev_sensor].num) + time) 
                              / (tv[prev_sensor].num + 1);
        tv[prev_sensor].num++;

        printf(2, "\033[s");
        if (i < 20) {
          printf(2, "\033[%d;40H", i + y);
        } else if (i < 40){
          printf(2, "\033[%d;70H", i % 20 + y);      
        } else if (i < 60){
          printf(2, "\033[%d;100H", i % 20 + y);      
        } else {
          printf(2, "\033[%d;130H", i % 20 + y);      
        }
        v = tv[i].length/tv[i].time;
        printf(2, "%s:%d t:%d n:%d sw:%d", 
                  track[prev_sensor].name, prev_sensor, 
                  tv[prev_sensor].time, tv[prev_sensor].num, d);
        
        printf(2, "\033[u");

        prev_sensor = i;

    }

    Reply(sender_tid, &result, sizeof(struct vel_request));
  }
}

