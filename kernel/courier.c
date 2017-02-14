#include "courier.h"
#include "user_syscall.h"
#include "clockserver.h"
#include "io.h"

int CR_TID = 0;
int CR_RV_TID = 0;

void set_train_speed(int train, int speed) {
  struct cr_request input;
  struct cr_request output;

  input.type = CR_SET_SPEED;
  input.arg1 = train;
  input.arg2 = speed;

  Send(CR_TID, &input, sizeof(struct cr_request), &output, sizeof(struct cr_request));
}

void reverse_train(int train) {
  struct cr_request input;
  struct cr_request output;

  input.type = CR_REVERSE_WAIT;
  input.arg1 = train;
  input.arg2 = 0;

  Send(CR_TID, &input, sizeof(struct cr_request), &output, sizeof(struct cr_request));

  Delay(500);

  input.type = CR_REVERSE_SET;
  input.arg1 = train;
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

  CR_TID = MyTid();

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
        Putc(1, req.arg2); // speed
        Putc(1, req.arg1); // train num
        break;

      case CR_REVERSE_WAIT:
        // set train speed to 0
        Putc(1, 0);
        Putc(1, req.arg1);

      case CR_REVERSE_SET:
        // reverse train
        Putc(1, 15);
        Putc(1, req.arg1);
        // set train speed to 14
        Putc(1, 14);
        Putc(1, req.arg1);
        break;

      case CR_SWITCH:
        Putc(1, req.arg2); // direction
        Putc(1, req.arg1); // switch num
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





