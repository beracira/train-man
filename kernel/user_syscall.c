#include "user_syscall.h"
#include "td.h"
#include "priorityqueue.h"
#include "../io/include/bwio.h"

int Create(int priority, void (*code) ( ) ) {
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  ks->syscall_code = 1;
  ks->args[0] = (int) priority;
  ks->args[1] = (int) code;

  asm("mov  ip, sp;");
  asm("stmfd  sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
  asm("swi 1;");

  return ks->usr_r0;
}

int MyTid(void) {
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  ks->syscall_code = 2;

  asm("mov  ip, sp;");
  asm("stmfd  sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
  asm("swi 2;");
  
  return ks->usr_r0;
}

int MyParentTid(void) {
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  ks->syscall_code = 3;

  asm("mov  ip, sp;");
  asm("stmfd  sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
  asm("swi 3;");

  return ks->usr_r0;
}

void Pass(void) {
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  ks->syscall_code = 4;
  
  asm("mov  ip, sp;");
  asm("stmfd  sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
  asm("swi 4;");

  return ks->usr_r0;
}


void Exit(void) {
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  ks->syscall_code = 5;
  
  asm("mov  ip, sp;");
  asm("stmfd  sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
  asm("swi 5;");

  return ks->usr_r0;
}
