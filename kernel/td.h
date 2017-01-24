#ifndef TD_H
#define TD_H

#include "priorityqueue.h"
#include "syscall.h"

#define ACTIVE 0
#define READY 1
#define ZOMBIE 2


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
};


// Order matters
struct kernel_stack {
  int * usr_sp;
  int * usr_lr;
  int usr_r0;
  int usr_spsr;
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
