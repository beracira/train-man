#ifndef TD_H
#define TD_H

#include "priorityqueue.h"
#include "syscall.h"

#define ACTIVE 0
#define READY 1
#define ZOMBIE 2
#define SEND_BLOCKED 3
#define RECEIVE_BLOCKED 4
#define REPLY_BLOCKED 5
#define EVENT_BLOCKED 6
#define WORKER_BLOCKED 21
#define SENSOR_BLOCKED 22
#define PATH_SWITCH_BLOCKED 23
#define GET_NEW_SENSOR_BLOCKED 31
#define PATH_SENSOR_BLOCKED 32

struct Sender {
  int sender_tid;
  void *msg; 
  int msglen;
  void *reply;
  int rplen;
};

struct task_descriptor {
  int tid;
  int state;
  int priority;
  int parent_tid;
  int * sp;
  int * lr;
  int return_value;
  int spsr;
  int started;
  int lr_svc;
  int irq;
  struct Sender sendq[MAX_TASKS];
  int sendq_first;
  int sendq_last; // update ks?
};


// Order matters
struct kernel_stack {
  int * usr_sp;
  int * usr_lr;
  int usr_r0;
  int usr_spsr;
  int lr_svc;
  int irq;
  int tid;
  int parent_tid;
  int priority;
  int num_tasks;
  int started;
  int syscall_code;
  int args[5];
};

int td_add(void * task, int priority, int parent_tid);
int set_active(int tid);
int sync_td(int tid);

#endif // TD_H
