#include "tc3_demo.h"
#include "common.h"
#include "td.h"
#include "train_client.h"
#include "clockserver.h"
#include "user_syscall.h"
#include "path_finding.h"
#include "track.h"
#include "train_ui.h"

int demo_mode = 0;
int DEMO_TID = 0;



void demo_tc3() {

  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  volatile track_node * track = (track_node *) 0x01700000;
  DEMO_TID = ks->tid;

  td[DEMO_TID].state = BLOCKED;
  Pass(); // sleep until unblocked by terminal_input_handlers

  demo_mode = 1;

  int sensor1_array[20];
  sensor1_array[0] = 63;
  sensor1_array[1] = 51;
  sensor1_array[2] = 74;
  sensor1_array[3] = 41;
  sensor1_array[4] = 33;
  sensor1_array[5] = 79;
  sensor1_array[6] = 37;
  sensor1_array[7] = 6;
  sensor1_array[8] = 47;
  sensor1_array[9] = 11;
  sensor1_array[10] = 0;
  sensor1_array[11] = 50;
  sensor1_array[12] = 77;
  sensor1_array[13] = 31;
  sensor1_array[14] = 73;
  sensor1_array[15] = 68;
  sensor1_array[16] = 24;
  sensor1_array[17] = 14;
  sensor1_array[18] = 43;
  sensor1_array[19] = 13;


  int sensor2_array[20];
  sensor2_array[0] = 21;
  sensor2_array[1] = 69;
  sensor2_array[2] = 69;
  sensor2_array[3] = 33;
  sensor2_array[4] = 29;
  sensor2_array[5] = 64;
  sensor2_array[6] = 16;
  sensor2_array[7] = 32;
  sensor2_array[8] = 39;
  sensor2_array[9] = 69;
  sensor2_array[10] = 77;
  sensor2_array[11] = 33;
  sensor2_array[12] = 13;
  sensor2_array[13] = 35;
  sensor2_array[14] = 79;
  sensor2_array[15] = 45;
  sensor2_array[16] = 19;
  sensor2_array[17] = 27;
  sensor2_array[18] = 53;
  sensor2_array[19] = 38;


  int i = 0;
  int sensor1, sensor2;

  while (1) {
    // td[DEMO_TID].state = BLOCKED; // or whatever
    // Pass();

    if (td[EVIL_WORKER_TID].state == WORKER_BLOCKED) {
      sensor1 = sensor1_array[i];
      td[EVIL_WORKER_TID].state = READY;
      train_64_struct.dest = sensor1;

      printf(2, "\033[s\033[23;40HNew evil dest: %s\033[u", track[sensor1].name);
      track_print_sensors(sensor1_array[(i-1)%20], 0);
      track_print_sensors(sensor1_array[i], 31);
    }

    Delay(100);

    if (td[OFFICER_WORKER_TID].state == WORKER_BLOCKED) {
      sensor2 = sensor2_array[i];
      td[OFFICER_WORKER_TID].state = READY;
      officer_struct.dest = sensor2;
      printf(2, "\033[s\033[24;40HNew just dest: %s\033[u", track[sensor2].name);
      track_print_sensors(sensor1_array[(i-1)%20], 0);
      track_print_sensors(sensor1_array[i], 33);
    }

    i = (i+1)%20;
    Delay(701);

  }
}
