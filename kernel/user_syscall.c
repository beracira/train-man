#include "user_syscall.h"
#include "td.h"
#include "priorityqueue.h"
#include "../io/include/bwio.h"
#include "io.h"

int Create(int priority, void (*code) ( ) ) {
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  ks->num_tasks += 1;
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
}


void Exit(void) {
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  ks->num_tasks -= 1;
  ks->syscall_code = 5;
  
  asm("mov  ip, sp;");
  asm("stmfd  sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
  asm("swi 5;");
}

int Send( int tid, void *msg, int msglen, void *reply, int rplen) {

  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  ks->syscall_code = 6;
  ks->args[0] = tid;
  ks->args[1] = (int) msg;
  ks->args[2] = msglen;
  ks->args[3] = (int) reply;
  ks->args[4] = rplen;

  asm("mov  ip, sp;");
  asm("stmfd  sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
  asm("swi 6;");

  return ks->usr_r0;

}

int Receive( int *tid, void *msg, int msglen ) {

  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;

  int my_tid = ks->tid;

  if (td[my_tid].sendq_first == td[my_tid].sendq_last) {
    td[my_tid].state = SEND_BLOCKED;
    Pass();
  }

  ks->syscall_code = 7;
  ks->args[0] = (int) tid;
  ks->args[1] = (int) msg;
  ks->args[2] = msglen;

  asm("mov  ip, sp;");
  asm("stmfd  sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
  asm("swi 7;");

  return ks->usr_r0;

}

int Reply( int tid, void *reply, int replylen ) {

  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  ks->syscall_code = 8;
  ks->args[0] = tid;
  ks->args[1] = (int) reply;
  ks->args[2] = replylen;

  asm("mov  ip, sp;");
  asm("stmfd  sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip};"); // save usr state
  asm("swi 8;");

  return ks->usr_r0;

}

int AwaitEvent(int eventid) {
  (void) eventid;
  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;

  int my_tid = ks->tid;
  td[my_tid].state = EVENT_BLOCKED;
  Pass();

  return 0;
}

void printf( int channel, char *fmt, ... ) {

  asm volatile ("sub sp, sp, #4");
  asm volatile ("str r0, [sp]");
  asm volatile ("mrs r0, cpsr");
  asm volatile ("sub sp, sp, #4");
  asm volatile ("str r0, [sp]");
  asm volatile ("ORR r0, r0, #0xc0");
  asm volatile ("msr cpsr, r0");
  asm volatile ("ldr r0, [sp, #-4]");

  va_list va;

  va_start(va,fmt);
  ioformat( channel, fmt, va );
  va_end(va);

  asm volatile ("str r0, [sp, #-4]");
  asm volatile ("ldr r0, [sp]");
  asm volatile ("add sp, sp, #8");
  asm volatile ("msr cpsr, r0");
  asm volatile ("ldr r0, [sp, #4]");
}

