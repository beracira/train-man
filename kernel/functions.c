#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

void firsttask(void) {

  bwprintf( COM2, "First user task created\n\r");
  
  int tid = 0;

  bwprintf( COM2, "Creating task: tid # %d\n\r", tid);
  tid = Create(P_LOW, the_other_task);
  bwprintf( COM2, "Creating task: tid # %d\n\r", tid);
  tid = Create(P_LOW, the_other_task);
  bwprintf( COM2, "Creating task: tid # %d\n\r", tid);
  tid = Create(P_HIGH, the_other_task);
  bwprintf( COM2, "Creating task: tid # %d\n\r", tid);
  tid = Create(P_HIGH, the_other_task);

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
