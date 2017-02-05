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

unsigned int idle_ticks = 0;

void idle_task(void) {
  int * timerLoad = (int *)(TIMER1_BASE + LDR_OFFSET);
  int * volatile timerValue = (int *)(TIMER1_BASE + VAL_OFFSET);
  int * timerControl = (int *)(TIMER1_BASE + CRTL_OFFSET);

  // 508000 cycles/s = 50800 cycles/0.1s
  int cyclesPerTick = ONE_TICK / 10;

  // Load the timer
  // Disable first
  *timerControl = (*timerControl) ^ ENABLE_MASK;
  // Load initial value
  *timerLoad = cyclesPerTick;
  // Enable timer, periodic mode, 508 kHz clock
  *timerControl = ENABLE_MASK | MODE_MASK| CLKSEL_MASK ;
  volatile int i = 0;
  (void) i;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  volatile int * temp = &(ks->num_tasks);
  idle_ticks = 0;
  unsigned int pre = cyclesPerTick;
  while (*temp != 4) {
    unsigned int cur = *((int *)(timerValue));
    if (cur > pre) ++idle_ticks;
    pre = cur;
  }
  bwprintf(COM2, "idle usage: %d%%\n\r", idle_ticks * 10 / time_ticks);
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
    bwprintf( COM2, "receive i: %d\n\r", i);
  }
  Exit();
}

void the_other_task_1(void){
  int my_tid = MyTid();
  bwprintf(COM2, "Task %d created at: %d\n\r", my_tid, Time());
  int i;
  for(i = 0; i < 20; ++i) {
    Delay(10);
    bwprintf(COM2, "Tid %d: delay time is %d number of delays is %d\n\r", my_tid, 10, i + 1);
    bwprintf(COM2, "idle usage: %d%%\n\r", idle_ticks * 10 / time_ticks);
  }
  bwprintf(COM2, "Task %d exiting at: %u\n\r", my_tid, Time());
  Exit();
}
void the_other_task_2(void){
  int my_tid = MyTid();
  bwprintf(COM2, "Task %d created at: %d\n\r", my_tid, Time());
  int i;
  for(i = 0; i < 9; ++i) {
    Delay(23);
    bwprintf(COM2, "Tid %d: delay time is %d number of delays is %d\n\r", my_tid, 23, i + 1);
    bwprintf(COM2, "idle usage: %d%%\n\r", idle_ticks * 10 / time_ticks);
  }
  bwprintf(COM2, "Task %d exiting at: %u\n\r", my_tid, Time());
  Exit();
}
void the_other_task_3(void){
  int my_tid = MyTid();
  bwprintf(COM2, "Task %d created at: %d\n\r", my_tid, Time());
  int i;
  for(i = 0; i < 6; ++i) {
    Delay(33);
    bwprintf(COM2, "Tid %d: delay time is %d number of delays is %d\n\r", my_tid, 33, i + 1);
    bwprintf(COM2, "idle usage: %d%%\n\r", idle_ticks * 10 / time_ticks);
  }
  bwprintf(COM2, "Task %d exiting at: %u\n\r", my_tid, Time());
  Exit();
}
void the_other_task_4(void){
  int my_tid = MyTid();
  bwprintf(COM2, "Task %d created at: %d\n\r", my_tid, Time());
  int i;
  for(i = 0; i < 3; ++i) {
    Delay(71);
    bwprintf(COM2, "Tid %d: delay time is %d number of delays is %d\n\r", my_tid, 71, i + 1);
    bwprintf(COM2, "idle usage: %d%%\n\r", idle_ticks * 10 / time_ticks);
  }
  bwprintf(COM2, "Task %d exiting at: %u\n\r", my_tid, Time());
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

