#ifndef TRAIN_UI_H
#define TRAIN_UI_H

#define UPDATE_TIME     1
#define UPDATE_IDLE     2
#define UPDATE_SWITCH   3
#define UPDATE_SENSORS  4

struct UI_Request{
  int type;
  int data1;
  int data2;
};

void UI_Server();

#endif // TRAIN_UI_H
