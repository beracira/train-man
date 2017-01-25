#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"
#include "kernel.h"
#include "syscall.h"
#include "functions.h"
#include "priorityqueue.h"
#include "td.h"
#include "user_syscall.h"

int activate(void);

void initialize(void) {

  bwsetfifo(COM2, OFF);
  bwprintf(COM2, "\n\r");

  // Load label for swi entry
  asm("ldr r0, =activate_enter_kernel;"); 
  asm("mov r1, #0x28;"); 
  asm("str r0, [r1, #0];");

  int first_tid = td_add(firsttask, P_MEDIUM, 0);
  set_active(first_tid);

  init_queue();
  
}

int activate(void) {

  // exiting kernel
  asm("mov	ip, sp;");
	asm("stmfd	sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr};"); // save svc state
  asm("mov r9, #0x01300000;");  // r9 = KERNEL_STACK_START
  asm("ldr r8, [r9, #12];");    // load spsr
  asm("msr spsr, r8;");         // change spsr

  asm("ldr r0, [r9, #8];");     // load return value to r0
  asm("ldr lr, [r9, #16];");     // load lr_svc
  asm("msr CPSR_c, #0xdf;");    // switch to system mode
  asm("ldr sp, [r9, #0];");     // load sp
  
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  if (ks->started == 1) {
    asm("ldmfd  sp, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp};");
  } else {
    ks->started = 1;
  }

  asm("msr CPSR_c, #0xd3;");    // back to supervisor mode
  asm("movs pc, lr;");          // let it go  


  // entering kernel
  asm("activate_enter_kernel:");
  asm("mov r9, #0x01300000;");  // r9 = KERNEL_STACK_START
  asm("str lr, [r9, #16];");     // save lr_svc to stack
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

int handle(int num) {
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  num = ks->syscall_code;
  // note: can't declare variables in switch statement
  int priority;
  int code; 

  switch(num){
    case CREATE:
      priority = ks->args[0];
      code = ks->args[1];
      ks->usr_r0 = kernel_Create(priority, (void *) code );
      break;
    case MYTID:
      ks->usr_r0 = kernel_MyTid();
      break;
    case MYPARENTPID:
      ks->usr_r0 =  kernel_MyParentTid();
      break;
    case PASS:
      ks->usr_r0 = kernel_Pass();
      break;
    case EXIT:
      ks->usr_r0 = kernel_Exit();
      break;
    default:
      return -1;
  }
  return 100;
}

int main( int argc, char* argv[] ) {

  initialize();

  while(1 + 1 == 2) {
    int active = schedule();
    if (active == -1) return 0;
    set_active(active);
    
    int request = activate(); //active);

    (void) request;
    handle(0);
    sync_td(active);
  }

  return 0;
}

