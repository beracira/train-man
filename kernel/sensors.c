#include "sensors.h"
#include "io.h"
#include "velocity.h"
#include "track.h"
#include "clockserver.h"

#define SENSOR_ARRAY_SIZE 10

int last_sensor = 0;

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

  //////////////////////////// 
  // velocity calibration

  struct train_velocity tv[80];

  int prev_time = 0;

  int time = 0;

  int prev_sensor = -1;
  volatile track_node * track = (track_node *) 0x01700000;

 // int i = -1;
  int d = 0;
  int y = 5;
  int k = 0;

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
      printf(2, "\033[s\033[26;30H\033[K");

      int i = (tail - 1 + SENSOR_ARRAY_SIZE) % SENSOR_ARRAY_SIZE;
      last_sensor = sensor_letter[i];
      last_sensor <<= 5;
      last_sensor += sensor_digit[i];
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

      //////////////////////////// 
      // velocity calibration
      if (prev_sensor == -1) {
          prev_sensor = last_sensor2;
          continue;
        } else if (prev_sensor == last_sensor2) {
          continue;
        }

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

        prev_sensor = last_sensor2;
        //////////////////////////// 
    }
  }
}
