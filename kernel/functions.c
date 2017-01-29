#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "nameserver.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

int strlen( char * str ) {
  int i = 0;
  while (str[i++]);
  return i;
}

void strcpy( char * origin, char * dest) {
  int i = 0;
  while (dest[i] = origin[i++]);
  return; 
}

int strcmp( char * str1, char * str2) {
  int i = 0;
  int len_1 = strlen(str1);
  int len_2 = strlen(str2);
  if (len_1 != len_2) return 0;
  while (i < len_1) 
    if (str1[i] != str2[i++]) return 0;
  return 1;
}

void nameserver(void);
void ns_test(void);
void ns_test1(void);

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

void firsttask(void) {

  //bwprintf( COM2, "First user task created\n\r");
  

  int tid = 0;
  char * a = "asdf";
  char b[5];
  bwprintf( COM2, "%d\n\r", strlen(a));
  strcpy(a, b);
  bwprintf( COM2, "%d\n\r", strlen(b));
  bwprintf( COM2, "%s\n\r", b);
  bwprintf( COM2, "%d\n\r", strcmp(a, b));
  bwprintf( COM2, "%d\n\r", strcmp("asdf", "fghj"));
  
  
  //bwprintf( COM2, "before nameserver create\n\r");
  tid = Create(P_MEDIUM, nameserver);
  //bwprintf( COM2, "after nameserver create\n\r");
  tid = Create(P_MEDIUM, ns_test);
  //bwprintf( COM2, "after test create\n\r");

  tid = Create(P_LOW, ns_test1);

  bwprintf(COM2, "%d \n\r", WhoIs("asd"));

  // tid = Create(P_LOW, dummy_sender);
  // bwprintf( COM2, "Creating task: tid # %d\n\r", tid);
  // tid = Create(P_MEDIUM, dummy_receiver);
  // bwprintf( COM2, "Creating task: tid # %d\n\r", tid);

  // bwprintf( COM2, "Exiting first user task.\n\r");
  Exit();
}

void the_other_task(void){
  bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());
  Pass();
  bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());

  bwprintf(COM2, "Exiting task (tid %d)\n\r", MyTid());
  Exit();
}

void nameserver(void) {
 // bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());
  volatile struct nameserver * ns = (struct namerserver *) NAME_SERVER_START;

  int i;
  int j;
  for (i = 0; i < MAX_NS_SIZE; ++i) {
    ns[i].tid = -1;
    ns[i].name[0] = 0;
  }

  while (1 + 1 == 2) {
    int sender_tid = 0;
    struct ns_request req;
    struct ns_request result;
    // bwprintf(COM2, "before receive \n\r");
    Receive( &sender_tid, &req, sizeof(struct ns_request));

    // bwprintf(COM2, "after receive \n\r");

    result.type = NS_TYPE_REPLY;

    if (req.type == NS_TYPE_REGISTERAS) {
      //bwprintf(COM2, "ns: registeras \n\r");
      for (i = 0; i < MAX_NS_SIZE; ++i) {
        // if (strcmp(ns[i].name, ))
        
        if (strcmp(ns[i].name, req.name)) {  // register previous used name
          ns[i].tid = req.tid;
          result.tid = req.tid;
          break;
        }
        if (ns[i].tid == -1) {  // never been used
          strcpy(req.name, ns[i].name);
          ns[i].tid = req.tid;
          result.tid = req.tid;
          break;
        }
      }
    }
    
    int whois_exists = 0;
    if (req.type == NS_TYPE_WHOIS) {
      //bwprintf(COM2, "ns: whois \n\r");
      for (i = 0; i < MAX_NS_SIZE; ++i) {
        if (strcmp(req.name, ns[i].name)) {
          result.tid = ns[i].tid;
          whois_exists = 1;
          break;
        }

        // for (j = 0; j < MAX_NAME_SIZE; j++){
        //   if (ns[i].name[j] != req.name[j]) {
        //     whois_exists = 0;
        //     break;
        //   }
        // }
        // if (whois_exists) {
        //    result.tid = ns[i].tid;
        //    break;
        // }
      }

      if (whois_exists == 0) {
        result.tid = -1;
      }
      
    }

    //bwprintf(COM2, "before reply \n\r");
    Reply(sender_tid, &result, sizeof(struct ns_request));
    //bwprintf(COM2, "after reply \n\r");
  }
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

void ns_test1(void) {
  bwprintf(COM2, "test2: before register as\n\r");
  RegisterAs("name4");
  int i = WhoIs("name4");
  bwprintf(COM2, "whois: %d\n\r", i);

  RegisterAs("name5");
  i = WhoIs("name5");
  bwprintf(COM2, "whois: %d\n\r", i);

  RegisterAs("name2");
  i = WhoIs("name2");
  bwprintf(COM2, "whois: %d\n\r", i);
  
  i = WhoIs("name1");
  bwprintf(COM2, "whois: %d\n\r", i);

  Exit();
}



