#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

void firsttask(void) {

  int tid = 0;

  tid = Create(P_LOW, the_other_task);
  bwprintf( COM2, "Created task: tid # %d\n\r", tid);
  tid = Create(P_LOW, the_other_task);
  bwprintf( COM2, "Created task: tid # %d\n\r", tid);
  tid = Create(P_HIGH, the_other_task);
  bwprintf( COM2, "Created task: tid # %d\n\r", tid);
  tid = Create(P_HIGH, the_other_task);
  bwprintf( COM2, "Created task: tid # %d\n\r", tid);
  bwprintf( COM2, "First user task: exiting.\n\r");
  Exit();
}

void the_other_task(void){
  bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());
  Pass();
  bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());
  Exit();
}
