#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

void firsttask(void) {

  bwprintf(COM2, "first1 task's tid is: %d\n\r", MyTid());
  bwprintf(COM2, "first2 task's parent's tid is: %d\n\r", MyParentTid());
  Create(P_HIGH, foo);
  bwprintf(COM2, "first3 task's tid is: %d\n\r", MyTid());
  bwprintf(COM2, "first4 task's parent's is: %d\n\r", MyParentTid());
  Exit();
  // bwprintf(COM2, "first1 task end\n\r");
  // Pass();
  // bwprintf(COM2, "first2 task end\n\r");
  // Pass();
  // bwprintf(COM2, "first3 task end:\n\r");
  // Pass();
  // bwprintf(COM2, "first4 task end:\n\r");
}

void foo(void){
  bwprintf(COM2, "please work \n\r");
  bwprintf(COM2, "foo's tid is: %d\n\r", MyTid());
  bwprintf(COM2, "foo's parent's tid is: %d\n\r", MyParentTid());
  Exit();
}
