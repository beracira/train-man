#include "td.h"
#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "nameserver.h"
#include "rps.h"
#include "common.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

void ns_test(void);
void rps_client(void);
void rps_client1(void);
void rps_client2(void);
void rps_client3(void);
void rps_client4(void);

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

  volatile int * temp = 0x800B0010;
  *temp = 1;
  temp = 0x800B0018;
  *temp = 1;

  bwprintf( COM2 ,"first_task: %d\n\r", (int) the_other_task );
  Create(P_HIGH, nameserver);
  Create(P_LOW, &the_other_task);
  timer_init();
  temp = 0x800B0018;
  *temp = 1;

  temp = 0x800B0014;
  *temp = 1;
  // volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;

  bwprintf( COM2 ,"%d\n\r", rand() );
  volatile int i = 0;
  while (i < 8000000) i++;
  bwprintf( COM2 ,"%d\n\r", rand() );
  // bwprintf( COM2 ,"%d\n\r", ks->usr_r0 );
  // bwprintf( COM2 ,"%d\n\r", ks->usr_spsr );
  // bwprintf( COM2 ,"%d\n\r", ks->lr_svc );
  // bwprintf( COM2 ,"%d\n\r", ks->irq );

  // volatile int i = 0;
  bwprintf(COM2, "after timer_init\n\r");

  Exit();
}

void the_other_task(void){
  bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());
  Pass();
  bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());

  volatile int * temp = 0x800B0010;
  *temp = 1;
  temp = 0x800B0018;
  *temp = 1;

  bwprintf(COM2, "Exiting task (tid %d)\n\r", MyTid());
  Exit();
}

void ns_test(void) {
  //bwprintf(COM2, "test: before register as\n\r");
  RegisterAs("name1");

  bwprintf(COM2, "test: after register as\n\r");
  int i = WhoIs("name1");
  bwprintf(COM2, "whois: %d\n\r", i);

  RegisterAs("name2");
  i = WhoIs("name2");
  bwprintf(COM2, "whois: %d\n\r", i);

  RegisterAs("name3");
  i = WhoIs("name3");
  bwprintf(COM2, "whois: %d\n\r", i);
  
  i = WhoIs("name2");
  bwprintf(COM2, "whois: %d\n\r", i);

  Exit();
}

void rps_client(void) {
  int i = -1; 
  int move = 0;

  while (1) {
    if (numtasks == 1) {
      break;    
    }
    move = (rand() % 3) + 1;
    rps_sign_up();

    i = rps_play(move);

   // bwprintf(COM2, "client i: %d \n\r", i);
    if (i == PLAYER_LOSE) {
      numtasks--;
      break;
    }
  }
  Exit();
}

void rps_client1(void){
  rps_sign_up();
  int i = rps_play(1);
  bwprintf(COM2, "client1 result: %d\n\r", i);

  rps_sign_up();
  i = rps_play(1);
  bwprintf(COM2, "client1 result: %d\n\r", i);
  rps_sign_up();
  i = rps_play(1);
  bwprintf(COM2, "client1 result: %d\n\r", i);
  rps_sign_up();
  i = rps_play(1);
  bwprintf(COM2, "client1 result: %d\n\r", i);

  Exit();
}

void rps_client2(void){
  rps_sign_up();
  rps_quit();
  Exit();
}

void rps_client3(void){
  rps_sign_up();
  int i = rps_play(3);
  bwprintf(COM2, "client3 result: %d\n\r", i);
  Exit();
}

void rps_client4(void){
  rps_sign_up();
  int i = rps_play(1);
  bwprintf(COM2, "client4 result: %d\n\r", i);
  rps_sign_up();
  i = rps_play(3);
  bwprintf(COM2, "client4 result: %d\n\r", i);
  Exit();
}
