#include "td.h"
#include "clockserver.h"
#include "user_syscall.h"
#include "courier.h"
#include "sensors.h"


#define MAX_QUEUE_SIZE 100

struct stop_wait_queue{
  int train_number[MAX_QUEUE_SIZE];
  int sensor[MAX_QUEUE_SIZE];
  int sensor_reached[MAX_QUEUE_SIZE];
  unsigned int time_reached[MAX_QUEUE_SIZE];
  unsigned int time_wait[MAX_QUEUE_SIZE];
};


static struct stop_wait_queue * queue_ptr = 0;
static int STOP_WORKER_TID = 0;
static volatile int next = 0;

void stop_worker() {
  STOP_WORKER_TID = MyTid();
  struct stop_wait_queue queue;
  queue_ptr = &queue;
  int i;
  for (i = 0; i < MAX_QUEUE_SIZE; ++i) {
    queue.train_number[i] = 0;
    queue.sensor[i] = -1;
    queue.sensor_reached[i] = 0;
    queue.time_reached[i] = 0;
    queue.time_wait[i] = 0;
  }
  next = -1;

  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  while (1 + 99 == 100) {
    td[ks->tid].state = WORKER_BLOCKED;
    Pass();
    if (next != -1) {
      set_train_speed(queue.train_number[next], 0);
      printf(2, "\033[s\033[H%d  \033[u", next); // back to top left
      queue.train_number[next] = 0;
      queue.sensor[next] = -1;
      queue.sensor_reached[next] = 0;
      queue.time_reached[next] = 0;
      queue.time_wait[next] = 0;
      next = -1;
    }
  }
}


// now only supports one train
void Stop(int train_number, int sensor, unsigned int time_wait) {
  int i;
  for (i = 0; i < MAX_QUEUE_SIZE; ++i) {
    if (queue_ptr->sensor[i] == -1) {
      queue_ptr->train_number[i] = train_number;
      queue_ptr->sensor[i] = sensor;
      queue_ptr->time_wait[i] = time_wait;
      break;
    }
  }
}

void remove_from_stop_queue() {
  int i;
  for (i = 0; i < MAX_QUEUE_SIZE; ++i) {
    if (queue_ptr->sensor[i] == last_sensor) {
      queue_ptr->sensor_reached[i] = 1;
      queue_ptr->time_reached[i] = time_ticks;
    }
  }
  // printf(2, "\033[s\033[H%d  \033[u", last_sensor);
  if (next == -1) {
    for (i = 0; i < MAX_QUEUE_SIZE; ++i) {
      if (queue_ptr->sensor_reached[i] 
        && queue_ptr->time_reached[i] + queue_ptr->time_wait[i] >= time_ticks) {
        next = i;
        break;
      }
    }
    if (next != -1) {
      volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
      td[STOP_WORKER_TID].state = READY;
    }
  }
}
