 /*
 * iotest.c
 */

#include <bwio.h>
#include <ts7200.h>

struct TD{
  int sp;
  int pc;
  int lr;
  //int reg[32];
  //TD * next;
  int tid;
//  int parent;
  int priority;
  int state;
  int exit_code;
};

// // 2 mb per task (roughly)
// TD td_list[1];
// int kernel_reg[32];

void syscall(void);

int handle(void) {

  bwprintf(COM2, "in handle\n\r");
  asm("ldmda sp, {r4,r5,r6,r7,r8,r9,r10,fp,ip,lr};");
  asm("mov sp, ip;");
  asm("bx lr;");

  // while(1);
  return 0;
}

int foo(void) { 
  bwprintf(COM2, "in foo\n\r");
  syscall();
  while(1);
  return 0;
}

// allocate all memory for user tasks. i.e. task_queue
// create a first user task, puts it into the queue
int init(void) {

  unsigned int sp;
  asm (
    "mov %0, SP;"
    : "=r" ( sp )
  );

  sp -= 4 * 1024 * 1024;

  // td_list[0].sp = sp;
  // td_list[0].pc = 0;
  // td_list[0].lr = 0;
  // td_list[0].tid = 1;
  // td_list[0].priority = 2;
  // td_list[0].state = 3;
  // td_list[0].exit_code = 4;

  // asm (
  //   "mov r12, %0;"
  //   : "=r" (sp)
  // );

  asm("mov r0, #0xd0;"); // user mode code to r0
  asm("msr SPSR, r0;"); // r0 to spsr
  asm("ldr lr, =handle;"); // load address of foo to lr

  asm("mov ip, sp;");  // store calculated sp to ip
  asm("msr CPSR_c, #0xdf;"); // switch to system mode
  asm("mov sp, ip;");  
  asm("msr CPSR_c, #0xd3;"); // back to supervisor mode

  asm("movs pc, lr;"); // let it go

  bwprintf( COM2, "In init.\n\r" ); 


  return 0;
}




// TD * schedule(void) {
//  int i = 0;
//  while (!task_queue[i]->ready) ++i;
//  return task_queue[i];
// }

// //context switch (loading registers)
// int activate (TD * td) {
//  // tbd
//  return 0;
// }

void syscall(void) {
  asm("swi 0;");
  asm("bx lr;");
}

int main( int argc, char* argv[] ) {
  int * entry = (int *) 0x28;
  *entry = 0x00218000;

  bwsetfifo(COM2, OFF);

  bwprintf(COM2, "\n\rHello World!\n\r");

  asm("mov ip, sp;");
  asm("stmib sp!, {r4,r5,r6,r7,r8,r9,r10,fp,ip,lr};");



  volatile unsigned int s = 0x00218000 + (int)foo;

  asm("mov r0, #0xd0;"); // user mode code to r0
  asm("msr SPSR, r0;"); // r0 to spsr
  asm("mov r0, %0" : "=r" (s));
  asm("mov lr, r0;"); // load address of foo to lr

  asm("mov ip, sp;");  // store calculated sp to ip
  asm("msr CPSR_c, #0xdf;"); // switch to system mode
  asm("mov sp, ip;");  
  asm("msr CPSR_c, #0xd3;"); // back to supervisor mode

  asm("movs pc, lr;"); // let it go

  // ldmfd r0!, {ip,lr} /* Get SPSR and lr */
  // msr SPSR, ip

  // mov sp, r0
  // pop {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,fp,ip,lr}

  bwprintf( COM2, "In init.\n\r" ); 


  return 0;
}

