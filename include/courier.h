#ifndef COURIER_H
#define COURIER_H

#define CR_SET_SPEED    1
#define CR_REVERSE_WAIT 2
#define CR_REVERSE_SET  3
#define CR_SWITCH       4
#define CR_QUIT         5
#define CR_SENSOR_REQUEST 451374

extern int courier_ready;
extern int CR_TID;

struct cr_request {
  int type;
  int arg1;
  int arg2;
};

struct CR_REVERSE_LIST {
  int train[100];
  int time[100];
  int speed[100];
  int head;
  int tail;
};

void set_train_speed(int train, int speed);

void reverse_train(int train);

void flip_switch(int switch_num, int dir);

void courier_server(void);

void wake_train();

void wake_train_second_part();

#endif /* COURIER_H */
