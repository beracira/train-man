#include "td.h"
#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "nameserver.h"
#include "clockserver.h"
#include "common.h"
#include "irq.h"
#include "kernel.h"

#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"


void idle_task(void) {
  volatile int i = 0;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  volatile int * temp = &(ks->num_tasks);
  while (*temp != 4) {
    while (i++ < 1600000);
    bwprintf(COM2, "%d%% %u %u\n\r", (idle_ticks * 100 / time_ticks), idle_ticks, time_ticks);
    i = 0;
  }
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
  int my_tid = MyTid();
  bwprintf(COM2, "Task %d created at: %d\n\r", my_tid, Time());
  int i;
  for(i = 0; i < 20; ++i) {
    Delay(100);
    // bwprintf(COM2, "Tid %d: delay time is %d number of delays is %d\n\r", my_tid, 10, i + 1);
  }
  bwprintf(COM2, "Task %d exiting at: %d\n\r", my_tid, Time());
  Exit();
}
void the_other_task_2(void){
  int my_tid = MyTid();
  bwprintf(COM2, "Task %d created at: %d\n\r", my_tid, Time());
  int i;
  for(i = 0; i < 9; ++i) {
    Delay(230);
    // bwprintf(COM2, "Tid %d: delay time is %d number of delays is %d\n\r", my_tid, 23, i + 1);
  }
  bwprintf(COM2, "Task %d exiting at: %d\n\r", my_tid, Time());
  Exit();
}
void the_other_task_3(void){
  int my_tid = MyTid();
  bwprintf(COM2, "Task %d created at: %d\n\r", my_tid, Time());
  int i;
  for(i = 0; i < 6; ++i) {
    Delay(330);
    // bwprintf(COM2, "Tid %d: delay time is %d number of delays is %d\n\r", my_tid, 33, i + 1);
  }
  bwprintf(COM2, "Task %d exiting at: %d\n\r", my_tid, Time());
  Exit();
}
void the_other_task_4(void){
  int my_tid = MyTid();
  bwprintf(COM2, "Task %d created at: %d\n\r", my_tid, Time());
  int i;
  for(i = 0; i < 3; ++i) {
    Delay(710);
    // bwprintf(COM2, "Tid %d: delay time is %d number of delays is %d\n\r", my_tid, 71, i + 1);
  }
  bwprintf(COM2, "Task %d exiting at: %d\n\r", my_tid, Time());
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

