#include "td.h"
#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "nameserver.h"
#include "clockserver.h"
#include "common.h"

#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"


void idle_task(void) {
  volatile int i;
  while (1 + 1 == 2) i++;

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

int numtasks = 8;

void firsttask(void) {


  bwprintf( COM2 ,"first_task: %d\n\r", (int) the_other_task );
  Create(P_HIGH, nameserver);
  Create(P_HIGH, clockserver);
  Create(P_MEDIUM, &the_other_task);
  Create(P_LOW, &idle_task);

  bwprintf(COM2, "Time is: %d \n\r", Time());
  DelayUntil(200);
  bwprintf(COM2, "Time is: %d \n\r", Time());


  bwprintf(COM2, "after timer_init\n\r");

  Exit();
}

void the_other_task(void){
  bwprintf(COM2, "Time is: %d \n\r", Time());

  DelayUntil(200);
  bwprintf(COM2, "Time is: %d \n\r", Time());

  bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());
  Pass();
  bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());

  bwprintf(COM2, "Time is: %d \n\r", Time());

  DelayUntil(500);
  bwprintf(COM2, "Time is: %d \n\r", Time());

  bwprintf(COM2, "Exiting task (tid %d)\n\r", MyTid());
  bwprintf(COM2, "Time is: %d \n\r", Time());

  DelayUntil(1000);
  bwprintf(COM2, "Time is: %d \n\r", Time());

  Exit();
}


