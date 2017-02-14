#ifndef COURIER_H
#define COURIER_H

#define CR_SET_SPEED    1
#define CR_REVERSE_WAIT 2
#define CR_REVERSE_SET  3
#define CR_SWITCH       4
#define CR_QUIT         5

struct cr_request {
  int type;
  int arg1;
  int arg2;
};


void set_train_speed(int train, int speed);

void reverse_train(int train);

void flip_switch(int switch_num, int dir);

void courier_server(void);

#endif /* COURIER_H */
