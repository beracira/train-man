#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"


#define SEC 508000
#define TIMER3_BASE 0x80810080
#define TIMER3_CONTROL 0x80810088
#define LDR_OFFSET  0x00000000  // 16/32 bits, RW
#define VAL_OFFSET  0x00000004  // 16/32 bits, RO
#define CRTL_OFFSET 0x00000008  // 3 bits, RW
#define TIME_PTR 0x80810084

int timer_init() {
  *((int *) (TIMER3_BASE + LDR_OFFSET)) = (SEC);
  *((int *) (TIMER3_CONTROL)) =  0x000000c8;
  return 0;
}

void dummy_sender_with_timer(void) {
  timer_init();
  char tid[64];
  char reply[64];
  int i;
  // bwprintf( COM2, "in sender %d\n\r", tid);
  for (i = 0; i < 100; ++i) {
    Send(3, &tid, 64, &reply, 64);
    // bwprintf( COM2, "send i: %d\n\r", i);
  }

  unsigned int cur = *((int *)(TIME_PTR));
  cur = 508000 - cur;
  cur *= 1000;
  cur /= 508000;
  bwprintf( COM2, "Time: %dms\n\r", cur);
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

void firsttask(void) {

  bwprintf( COM2, "First user task created\n\r");
  
  Create(P_MEDIUM, dummy_sender_with_timer);
  Create(P_LOW, dummy_receiver_with_timer);

  Exit();
}

void the_other_task(void){
  bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());
  Pass();
  bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());

  bwprintf(COM2, "Exiting task (tid %d)\n\r", MyTid());
  Exit();
}


