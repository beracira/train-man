#include "td.h"

int td_counter = 1;

int td_add(void * task, priority_t priority, int parent_tid) {
  struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  td[td_counter].tid = td_counter;
  td[td_counter].state = READY;
  td[td_counter].priority = priority;
  td[td_counter].parent_tid = parent_tid;
  td[td_counter].sp = (int *) USER_STACK_START - USER_STACK_SIZE * td_counter;
  td[td_counter].lr = (int *) task;
  td[td_counter].return_value = 0;
  td[td_counter].spsr = 0xd0;
  td[td_counter].started = 0;

  return td_counter++;
}

int set_active(int tid) {
  struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  ks->usr_sp = td[tid].sp;
  ks->usr_lr  = td[tid].lr;
  ks->usr_r0 = td[tid].return_value;
  ks->usr_spsr = td[tid].spsr;
  ks->num_tasks = 1;
  ks->started = td[tid].started;
  return 0;
}

int sync_td(int tid) {
  struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  td[tid].sp = ks->usr_sp;
  td[tid].lr = ks->usr_lr;
  td[tid].return_value = ks->usr_r0;
  td[tid].spsr = ks->usr_spsr;
  td[tid].started = ks->started;
  return 0;
}
