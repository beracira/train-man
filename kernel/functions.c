#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

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

void dummy_receiver(void) {
  int sender_tid = -1;
  char msg[30];
  bwprintf( COM2, "before receive\n\r");
  int result = Receive(&sender_tid, msg, 30);
  bwprintf( COM2, "after receive\n\r");
  bwprintf( COM2, "Result of receive %d\n\r", result);
  bwprintf( COM2, "%s\n\r", msg);
  char * msg_ = "leave me alone plz";
  result = Reply(sender_tid, msg_, 30);
  bwprintf( COM2, "Result of reply: %d\n\r", result);
  Exit();
}

void firsttask(void) {

  bwprintf( COM2, "First user task created\n\r");
  
  int tid = 0;

  tid = Create(P_LOW, dummy_sender);
  bwprintf( COM2, "Creating task: tid # %d\n\r", tid);
  tid = Create(P_MEDIUM, dummy_receiver);
  bwprintf( COM2, "Creating task: tid # %d\n\r", tid);

  bwprintf( COM2, "Exiting first user task.\n\r");
  Exit();
}

void the_other_task(void){
  bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());
  Pass();
  bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());

  bwprintf(COM2, "Exiting task (tid %d)\n\r", MyTid());
  Exit();
}


