#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"
#include "kernel.h"
#include "contextswitch.h"
#include "functions.h"

void initialize(void) {

  // // set up swi handler
  // int * swi_vector = (int *)(0x8);
  // int * handler = (int *)(0x28);
  // *swi_vector = 0xe590f018;
  // *handler = (int)&swi_handler;

  int * first_swi_instruction_address = (int *)(0x8);
	int * swi_call_entry_address = (int *)(0x20);
	void (*swi_entry)() = swi_handler;
	*swi_call_entry_address = (int)swi_entry;
	// 1110(always) 010 (load/store immediate ) 1(offset based)  0(subtracted from base) 0(word access) 0(not updating base) 1(load) 1111(r15) 1111(r15) 0000 0000 0000 
	//
	/*  From the doc: 'If R15 is specified as register Rn, the value used is the address of the instruction plus eight.
	 *  ' WHAT??? WHY??? I WASTED A WHOLE DAY BECAUSE OF THIS!!!
	 *
	 * LRD PC [PC, #-4]*/
	*first_swi_instruction_address = 0xE51FF004;
	first_swi_instruction_address++;
	//  Put the address of the fcn here
	*first_swi_instruction_address = (int)&swi_handler;

  struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;

  //struct priority_queue * pq = *((struct priority_queue **) PRIORITY_QUEUE_START);
  struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  td[0].tid = 1;
  td[0].state = 5;
  td[0].priority = 1;
  td[0].parent_tid = 2;
  td[0].sp = (int *) USER_STACK_START;
  td[0].lr = (int *) &firsttask;
  td[0].return_value = 0;
  td[0].spsr = 16;
  ks->current_td = (int *) td;
  ks->usr_sp = td[0].sp;
	ks->usr_lr  = td[0].lr;
	ks->usr_r0 = td[0].return_value;
  ks->usr_spsr = td[0].spsr;
  ks->num_tasks = 1;
  exit_kernel();
}

int main( int argc, char* argv[] ) {

  // main starts in svc mode, where bwprintf doesn't work

  initialize();
  return 0;
}

