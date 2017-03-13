#ifndef TRAIN_UI_H
#define TRAIN_UI_H

#define UPDATE_TIME     1
#define UPDATE_IDLE     2
#define UPDATE_SWITCH   3
#define UPDATE_SENSORS  4

extern int ui_ready;
extern int UI_TID;

struct UI_Request{
  int type;
  int data1;
  int data2;
};

int update_switch(int switch_number, int direction);

void UI_Server();

int update_switch(int switch_number, int direction);

#endif // TRAIN_UI_H
