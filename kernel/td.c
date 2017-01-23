int td_counter = 0;

int td_add(void * task, int priority, int parent_tid) {
  task_descriptor * td = TASK_DESCRIPTOR_START;
  td += td_counter;
  td->tid = td_counter;
  td->state = READY;
  td->priority = priority;
  td->parent_tid = parent_tid;
  td->sp = USER_STACK_START - USER_STACK_SIZE * td_counter;
  td->lr = (int *) task;
  td->return_value = 0;
  td->spsr = 0xd0;
  td->started = 0;

  return td_counter++;
}

int set_active(int tid) {
  struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  task_descriptor * td = TASK_DESCRIPTOR_START;
  td += tid;
  ks->current_td = (int *) td;
  ks->usr_sp = td->sp;
  ks->usr_lr  = td->lr;
  ks->usr_r0 = td->return_value;
  ks->usr_spsr = td->spsr;
  ks->num_tasks = 1;
  ks->started = 0;
}