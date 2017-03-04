#include "velocity.h"
#include "clockserver.h"
#include "user_syscall.h"
#include "track.h"
#include "common.h"

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
  track_node * track = (track_node *) 0x01700000;

  int i;

  printf(2, "\033[s");
  for (i = 0; i < 80; i++) {
    tv[i].length = track[i].edge[DIR_AHEAD].dist;
    tv[i].num = 0;
    tv[i].time = 0;
    v = tv[i].length/tv[i].time;
    if (i < 40) {
      printf(2, "\033[%d;40H", i % 40 + 50);
    } else {
      printf(2, "\033[%d;100H", i % 40 + 50);      
    }
    printf(2, "%d: t = %d, d = %d, v = %d, # = %d \n\r", i, tv[i].time, tv[i].length, v, tv[i].num);
  }
  printf(2, "\033[u");

  while(1) {
    Receive( &sender_tid, &req, sizeof(struct vel_request));

    switch(req.type) {
      case SENSOR_ACTIVATED:

        i = req.sensor;

        if (prev_sensor == i) {
          break;
        }
        time = time_ticks - prev_time;
        prev_time = time_ticks;
        tv[req.sensor].time = ((tv[req.sensor].time * tv[req.sensor].num) + time) / (tv[req.sensor].num + 1);
        tv[req.sensor].num++;

        printf(2, "\033[s");
        if (i < 40) {
          printf(2, "\033[%d;40H", i % 40 + 50);
        } else {
          printf(2, "\033[%d;100H", i % 40 + 50);      
        }

        v = tv[i].length/tv[i].time;
        printf(2, "%d - %d: t = %d, d = %d, v = %d, # = %d \n\r", prev_sensor, i, tv[i].time, tv[i].length, v, tv[i].num);
        
        printf(2, "\033[u");
        prev_sensor = i;
        break;
      default:
        break;
    }
    Reply(sender_tid, &result, sizeof(struct vel_request));
  }
}

