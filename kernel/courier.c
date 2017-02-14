#include "courier.h"
#include "user_syscall.h"
#include "clockserver.h"
#include "io.h"

int CR_TID = 0;

void set_train_speed(int train, int speed) {
  struct cr_request input;
  struct cr_request output;

  input.type = CR_SET_SPEED;
  input.arg1 = train;
  input.arg2 = speed;

  printf(2, "cr tid: %d \n\r", CR_TID);

  Send(CR_TID, &input, sizeof(struct cr_request), &output, sizeof(struct cr_request));
}

void reverse_train(int train) {
  struct cr_request input;
  struct cr_request output;

  input.type = CR_REVERSE_WAIT;
  input.arg1 = train;
  input.arg2 = 0;

  Send(CR_TID, &input, sizeof(struct cr_request), &output, sizeof(struct cr_request));
}

void wake_train() {
  struct cr_request input;
  struct cr_request output;

  input.type = CR_REVERSE_SET;
  input.arg1 = 0;
  input.arg2 = 0;

  Send(CR_TID, &input, sizeof(struct cr_request), &output, sizeof(struct cr_request));
}

void flip_switch(int switch_num, int dir) {
  struct cr_request input;
  struct cr_request output;

  input.type = CR_SWITCH;
  input.arg1 = switch_num;
  input.arg2 = dir;

  Send(CR_TID, &input, sizeof(struct cr_request), &output, sizeof(struct cr_request));
}

void quit_system(void) {
  struct cr_request input;
  struct cr_request output;

  input.type = CR_QUIT;
  input.arg1 = 0;
  input.arg2 = 0;

  Send(CR_TID, &input, sizeof(struct cr_request), &output, sizeof(struct cr_request));
}

void courier_server(void) {

  int i = 0;

  CR_TID = MyTid();
  int train_list[100];
  
  for (i = 0; i < 100; ++i) {
    train_list[i] = -1;
  }

  struct CR_REVERSE_LIST cr_reverse_list;
  cr_reverse_list.head = 0;
  cr_reverse_list.tail = 0;

  int first_temp = 0;

  for (i = 0; i < 100; ++i) {
    cr_reverse_list.train[i] = -1;
    cr_reverse_list.time[i] = -1;
  }

  int sender_tid = -1;
  struct cr_request req;
  struct cr_request result;
  result.type = 0;
  result.arg1 = 0;
  result.arg2 = 0;

  while (1) {
    Receive( &sender_tid, &req, sizeof(struct cr_request));

    switch(req.type) {
      case CR_SET_SPEED:
        printf(1, "%c%c", req.arg2, req.arg1);
        train_list[req.arg1] = req.arg2;
        break;

      case CR_REVERSE_WAIT:
        if (train_list[req.arg1] == 0) { // train is already reversing
          break;
        } else {
          // set train speed to 0
          printf(1, "%c%c", 0, req.arg1);
          cr_reverse_list.speed[cr_reverse_list.tail] = train_list[req.arg1];
          cr_reverse_list.train[cr_reverse_list.tail] = req.arg1;
          cr_reverse_list.time[cr_reverse_list.tail++] = 500;
          cr_reverse_list.tail %= 100;
          train_list[req.arg1] = 0;
        }
        
      case CR_REVERSE_SET:

        if (cr_reverse_list.time[cr_reverse_list.head] == 0) {
          // reverse train
          printf(1, "%c%c", 15, cr_reverse_list.train[cr_reverse_list.head]);
          // set train speed to 14
          printf(1, "%c%c", cr_reverse_list.speed[cr_reverse_list.head], req.arg1);

          // remove from wait list
          train_list[req.arg1] = cr_reverse_list.speed[cr_reverse_list.head];
          cr_reverse_list.speed[cr_reverse_list.head] = -1;
          cr_reverse_list.train[cr_reverse_list.head] = -1;
          cr_reverse_list.time[cr_reverse_list.head++] = -1;
          cr_reverse_list.head %= 100;
        }
        first_temp = cr_reverse_list.head;

        while (first_temp != cr_reverse_list.tail) {
          if (cr_reverse_list.train[first_temp] == -1) {
            break;
          } else {
            cr_reverse_list.time[first_temp] -= 1;
            first_temp++;
          }
        }


        break;

      case CR_SWITCH:
        printf(1, "%c%c", req.arg2, req.arg1);
        Putc(1, 32);
        break;

      case CR_QUIT:
        break;

      default:
        break;
    }
    Reply(sender_tid, &result, sizeof(struct cr_request));
  }
}





