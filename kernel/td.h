#ifndef TD_H
#define TD_H

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
  int num_tasks;
  int started;

  int * current_td;
};


#endif // TD_H
