#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"
#include "kernel.h"
#include "contextswitch.h"
#include "functions.h"

void initialize(void) {
  struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;

  //struct priority_queue * pq = *((struct priority_queue **) PRIORITY_QUEUE_START);
  struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  td->tid = 1;
  td[0].state = 5;
  td[0].priority = 1;
  td[0].parent_tid = 2;
  td[0].sp = (int *) USER_STACK_START;
  td[0].lr = (int *) &firsttask;
  td[0].return_value = 0;
  ks->current_td = (int *) td;
  ks->usr_sp = td[0].sp;
	ks->usr_lr  = td[0].lr;
	ks->usr_r0 = td[0].return_value;
  ks->num_tasks = 1;
  exit_kernel();
}

int main( int argc, char* argv[] ) {

 // volatile unsigned int s1 = 0x00218000 + (int)firsttask;

  bwsetfifo( COM2, OFF );
bwprintf(COM2, "main\n\r");

  initialize();
  return 0;
}

