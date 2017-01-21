#ifndef CONTEXTSWITCH_H
#define CONTEXTSWITCH_H

#define TASK_DESCRIPTOR_START 0x01400000
#define PRIORITY_QUEUE_START  0x01600000
#define KERNEL_STACK_START    0x01300000
#define USER_STACK_START 	  0x01200000
#define USER_STACK_SIZE       0x00010000

#define MAX_TASKS 16

struct task_descriptor {
  int tid;
  int state;
  int priority;
  int parent_tid;
  int * sp;
  int * lr;
  int return_value;
  int spsr;
};

// struct queue {
//   int tid[16];
//   int first;
//   int last;
// };

// struct priority_queue {
//   struct queue high;
//   struct queue mid;
//   struct queue low;
// }

// Order matters
struct kernel_stack {
	int * usr_sp;
	int * usr_lr;
	int usr_r0;
  int usr_spsr;
  int num_tasks;
  int * current_td;
};

void syscall(int code);

void handle(int code);

void exit_kernel(void);

void initialize(void);

void swi_handler(void);

#endif /* CONTEXTSWITCH_H */
