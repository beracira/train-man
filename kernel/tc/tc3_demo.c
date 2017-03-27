#include "tc3_demo.h"
#include "common.h"
#include "td.h"
#include "train_client.h"
#include "clockserver.h"
#include "user_syscall.h"
#include "path_finding.h"

int demo_mode = 0;
int DEMO_TID = 0;

void demo_tc3() {

  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  DEMO_TID = ks->tid;

  td[DEMO_TID].state = WORKER_BLOCKED;
  Pass(); // sleep until unblocked by terminal_input_handlers

  demo_mode = 1;


  int sensor1 = rand() % 80;
  while (sensor1 == 26 || sensor1 == 27) sensor1 = rand() % 80;
  if (td[EVIL_WORKER_TID].state == WORKER_BLOCKED) {
    td[EVIL_WORKER_TID].state = READY;
    train_64_struct.dest = sensor1;
  }

  Delay(10);

  int sensor2 = rand() % 80;
  while (sensor2 == sensor1 || sensor2 == 22 || sensor2 == 23) sensor2 = rand() % 80;
  if (td[OFFICER_WORKER_TID].state == WORKER_BLOCKED) {
    td[OFFICER_WORKER_TID].state = READY;
    officer_struct.dest = sensor2;
  }

  while (1) {
    td[DEMO_TID].state = WORKER_BLOCKED; // or whatever
    Pass();

    Delay(500);

    if (td[EVIL_WORKER_TID].state == WORKER_BLOCKED) {
      sensor1 = rand() % 80;
      td[EVIL_WORKER_TID].state = READY;
      train_64_struct.dest = sensor1;
    }

    if (td[OFFICER_WORKER_TID].state == WORKER_BLOCKED) {
      sensor2 = rand() % 80;
      td[OFFICER_WORKER_TID].state = READY;
      officer_struct.dest = sensor2;
    }

  }
}
