#include "user_syscall.h"

int Create(priority_t priority, void (*code) ( ) ) {
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  ks->syscall_code = 1;
  ks->args[0] = (int) priority;
  ks->args[1] = (int) code;

  asm("mov  ip, sp;");
  asm("stmfd  sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
  asm("swi 0;");

  return -1;
}

int MyTid(void) {
  asm("mov  ip, sp;");
  asm("stmfd  sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
  asm("swi 1;");

  struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  return ks->usr_r0;
}

int MyParentTid(void);

void Pass(void);

void Exit(void);
