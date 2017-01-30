#include "nameserver.h"
#include "user_syscall.h"
#include "syscall.h"
#include "td.h"
#include "common.h"

/*
Returns
0 Success.
-1 The nameserver task id inside the wrapper is invalid.
*/
int RegisterAs( char *name ) {

  struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;

  struct ns_request input;
  struct ns_request output;

  input.type = NS_TYPE_REGISTERAS;
  input.name = name;
  input.tid = ks->tid;

  Send(NS_TID, &input, sizeof(struct ns_request), &output, sizeof(struct ns_request));

  // todo error checking 
  return 0;
}

/*
Returns
tid The task id of the registered task.
-1 The nameserver task id inside the wrapper is invalid.
*/
int WhoIs( char *name ) {

  struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;

  struct ns_request input;
  struct ns_request output;

  input.type = NS_TYPE_WHOIS;
  input.name = name;
  input.tid = ks->tid;

  Send(NS_TID, &input, sizeof(struct ns_request), &output, sizeof(struct ns_request));

  return output.tid;

}

void nameserver(void) {
 // bwprintf(COM2, "My tid is: %d. My parent's tid is: %d.\n\r", MyTid(), MyParentTid());
  volatile struct nameserver * ns = (struct nameserver *) NAME_SERVER_START;

  int i;
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
        
        if (strcmp((char *) ns[i].name, (char *) req.name)) {  // register previous used name
          ns[i].tid = req.tid;
          result.tid = req.tid;
          break;
        }
        if (ns[i].tid == -1) {  // never been used
          strcpy(req.name, (char *)ns[i].name);
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
        if (strcmp(req.name, (char *)ns[i].name)) {
          result.tid = ns[i].tid;
          whois_exists = 1;
          break;
        }
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
