#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"
#include "kernel.h"
#include "contextswitch.h"
#include "functions.h"

int activate(void);

void initialize(void) {

 // int * swi_vector = (int *)(0x8);
//	int * handler = (int *)(0x20);
//	void (*swi_entry)() = swi_handler;
//	*handler = (int)swi_entry;

//	*swi_vector = 0xE51FF004; // 0xe590f018;
	//swi_vector++;
	//  Put the address of the fcn here
//*swi_vector = (int)&swi_handler;

  asm("ldr r0, =activate_enter_kernel;"); 
  asm("mov r1, #0x28;"); 
  asm("str r0, [r1, #0];");

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
  td[0].spsr = 0xD0;
  td[0].started = 0;
  ks->current_td = (int *) td;
  ks->usr_sp = td[0].sp;
	ks->usr_lr  = td[0].lr;
	ks->usr_r0 = td[0].return_value;
  ks->usr_spsr = td[0].spsr;
  ks->num_tasks = 1;
  ks->started = 0;
  activate();
}

int activate(void) {

  // exiting kernel
  asm("mov	ip, sp;");
	asm("stmfd	sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr};"); // save svc state
  asm("mov r9, #0x01300000;");  // r9 = KERNEL_STACK_START
  asm("ldr r8, [r9, #12];");    // load spsr
  asm("msr spsr, r8;");         // change spsr
  asm("ldr lr, [r9, #4];");     // load lr
  asm("ldr r0, [r9, #8];");     // load return value to r0
  asm("msr CPSR_c, #0xdf;");    // switch to system mode
  asm("ldr sp, [r9, #0];");     // load sp
  struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  if (ks->started == 1) {
    asm("ldmfd	sp, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp};");
  }
  else {
    ks->started = 1;
  }
  asm("msr CPSR_c, #0xd3;");    // back to supervisor mode
  asm("movs pc, lr;");          // let it go  

  // entering kernel
  asm("activate_enter_kernel:");
  asm("mov r9, #0x01300000;");  // r9 = KERNEL_STACK_START
  asm("msr CPSR_c, #0xDF;");      // switch to system mode
  asm("str sp, [r9, #0];");     // store sp
  asm("str lr, [r9, #4];");     // save lr to stack
  asm("msr CPSR_c, #0xD3;");      // back to svc
  asm("mrs r8, spsr;");         // move spsr to r8
  asm("str r8, [r9, #12];");    // store spsr
  //asm("mov sp, r9;");           // move kernel stack pointer to sp
  asm("ldmfd	sp, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr};"); // loads svc state
  asm("ldr r0, [lr,#-4];");     // load swi code 
  asm("bic r0, r0, #0xff000000;");  // get number
  //asm("bl handle;");
  //get r0 into an int

  return 0;
}

int main( int argc, char* argv[] ) {

  // main starts in svc mode, where bwprintf doesn't work
  initialize();

 // activate();


  // while(1) {
  //   //active = schedule( );
  //   int request = activate(); //active);
  //   handle(request);
  // }

  return 0;
}

