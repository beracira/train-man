#include "td.h"
#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "nameserver.h"
#include "clockserver.h"
#include "common.h"
#include "irq.h"

#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"


void idle_task(void) {
  volatile int i;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  volatile int * temp = &(ks->num_tasks);
  while (*temp != 4); 
  Exit();
}

void dummy_sender(void) {
  char * str = "this is a lot of work";
  char reply[30];
  bwprintf( COM2, "before send\n\r");
  int result = Send(3, str, 22, reply, 30);
  bwprintf( COM2, "after send\n\r");
  bwprintf( COM2, "Result of send: %d\n\r", result);
  bwprintf( COM2, "%s\n\r", reply);
  Exit();
}

void dummy_receiver_with_timer(void) {

  int sender_tid = -1;
  char msg[64];
  int i;
  for (i = 0; i < 100; ++i) {
    Receive(&sender_tid, &msg, 64);
    Reply(sender_tid, &msg, 64);
    // bwprintf( COM2, "receive i: %d\n\r", i);
  }
  Exit();
}

void the_other_task_1(void){
  // bwprintf(COM2, "Task 1 created\n\r");
  bwprintf(COM2, "Task 1 created at: %d\n\r", Time());
  int i;
  for(i = 0; i < 20; ++i) Delay(10);
  // bwprintf(COM2, "Task 1 exiting\n\r");
  bwprintf(COM2, "Task 1 exiting at: %d\n\r", Time());
  Exit();
}
void the_other_task_2(void){
  // bwprintf(COM2, "Task 2 created\n\r");
  bwprintf(COM2, "Task 2 created at: %d\n\r", Time());
  int i;
  for(i = 0; i < 9; ++i) Delay(23);
  // bwprintf(COM2, "Task 2 exiting\n\r");
  bwprintf(COM2, "Task 2 exiting at: %d\n\r", Time());
  Exit();
}
void the_other_task_3(void){
  // bwprintf(COM2, "Task 3 created\n\r");
  bwprintf(COM2, "Task 3 created at: %d\n\r", Time());
  int i;
  for(i = 0; i < 6; ++i) Delay(33);
  // bwprintf(COM2, "Task 3 exiting\n\r");
  bwprintf(COM2, "Task 3 exiting at: %d\n\r", Time());
  Exit();
}
void the_other_task_4(void){
  // bwprintf(COM2, "Task 4 created\n\r");
  bwprintf(COM2, "Task 4 created at: %d\n\r", Time());
  int i;
  for(i = 0; i < 3; ++i) Delay(71);
  // bwprintf(COM2, "Task 4 exiting\n\r");
  bwprintf(COM2, "Task 4 exiting at: %d\n\r", Time());
  Exit();
}

void firsttask(void) {

  Create(P_HIGH, nameserver);
  Create(P_HIGH, clockserver);
  Create(P_SUPER_HIGH, timer_notifier);
  Create(P_LOW, &idle_task);

  Create(3, &the_other_task_1);
  Create(4, &the_other_task_2);
  Create(5, &the_other_task_3);
  Create(6, &the_other_task_4);

  Exit();
}

