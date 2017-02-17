#include "courier.h"
#include "user_syscall.h"
#include "clockserver.h"
#include "io.h"
#include "train_ui.h"
#include "sensors.h"

int CR_TID = 0;
int courier_ready = 0;

struct CR_REVERSE_LIST * cr_reverse_list_ptr = 0;
struct CR_REVERSE_LIST * cr_set_speed_list_ptr = 0;
int * train_list_ptr = 0;


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
}

void wake_train() {

  int first_temp = 0;
  if (cr_reverse_list_ptr->time[cr_reverse_list_ptr->head] == 0) {
    // reverse train
    // printf(1, "%c%c", 15, cr_reverse_list_ptr->train[cr_reverse_list_ptr->head]);
    set_train_speed(cr_reverse_list_ptr->train[cr_reverse_list_ptr->head], 15);
    // set train speed to 14
    cr_set_speed_list_ptr->speed[cr_set_speed_list_ptr->tail] = cr_reverse_list_ptr->speed[cr_reverse_list_ptr->head];
    cr_set_speed_list_ptr->train[cr_set_speed_list_ptr->tail] = cr_reverse_list_ptr->train[cr_reverse_list_ptr->head];
    cr_set_speed_list_ptr->time[cr_set_speed_list_ptr->tail++] = time_ticks;

    // remove from wait list
    cr_reverse_list_ptr->speed[cr_reverse_list_ptr->head] = -1;
    cr_reverse_list_ptr->train[cr_reverse_list_ptr->head] = -1;
    cr_reverse_list_ptr->time[cr_reverse_list_ptr->head++] = -1;
    cr_reverse_list_ptr->head %= 100;
  }
  first_temp = cr_reverse_list_ptr->head;

  while (first_temp != cr_reverse_list_ptr->tail) {
    if (cr_reverse_list_ptr->train[first_temp] == -1) {
      break;
    } else {
      cr_reverse_list_ptr->time[first_temp] -= 1;
      first_temp++;
    }
  }

  if (cr_set_speed_list_ptr->head == cr_set_speed_list_ptr->tail) return;
  if (time_ticks - cr_set_speed_list_ptr->time[cr_set_speed_list_ptr->head] > 200) {
    train_list_ptr[cr_set_speed_list_ptr->train[cr_set_speed_list_ptr->head]] = cr_set_speed_list_ptr->speed[cr_set_speed_list_ptr->head];
    // printf(1, "%c%c", cr_set_speed_list_ptr->speed[cr_set_speed_list_ptr->head], cr_set_speed_list_ptr->train[cr_set_speed_list_ptr->head]);
    set_train_speed(cr_set_speed_list_ptr->train[cr_set_speed_list_ptr->head],
      cr_set_speed_list_ptr->speed[cr_set_speed_list_ptr->head]);
    cr_set_speed_list_ptr->head++;
    cr_set_speed_list_ptr->head %= 100;
  }
}

void wake_train_second_part() {
  if (cr_set_speed_list_ptr->head == cr_set_speed_list_ptr->tail) return;
  if (time_ticks - cr_set_speed_list_ptr->time[cr_set_speed_list_ptr->head] > 200) {
    train_list_ptr[cr_set_speed_list_ptr->train[cr_set_speed_list_ptr->head]] = cr_set_speed_list_ptr->speed[cr_set_speed_list_ptr->head];
    printf(1, "%c%c", cr_set_speed_list_ptr->speed[cr_set_speed_list_ptr->head], cr_set_speed_list_ptr->train[cr_set_speed_list_ptr->head]);
    cr_set_speed_list_ptr->head++;
    cr_set_speed_list_ptr->head %= 100;
  }
}

void flip_switch(int switch_num, int dir) {
  struct cr_request input;
  struct cr_request output;

  if (ui_ready) {
    struct UI_Request input;
    struct UI_Request output;
    input.type = UPDATE_SWITCH;
    input.data1 = switch_num;
    input.data2 = dir;

    Send(UI_TID, &input, sizeof(struct UI_Request), &output, sizeof(struct UI_Request));
  }

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
  struct CR_REVERSE_LIST cr_set_speed_list;
  cr_reverse_list.head = 0;
  cr_reverse_list.tail = 0;
  cr_set_speed_list.head = 0;
  cr_set_speed_list.tail = 0;

  for (i = 0; i < 100; ++i) {
    cr_reverse_list.train[i] = -1;
    cr_reverse_list.time[i] = -1;
    cr_set_speed_list.train[i] = -1;
    cr_set_speed_list.time[i] = -1;
  }

  train_list_ptr = train_list;
  cr_reverse_list_ptr = &cr_reverse_list;
  cr_set_speed_list_ptr = &cr_set_speed_list;

  int sender_tid = -1;
  struct cr_request req;
  struct cr_request result;
  char dummy;
  int request = 0;
  result.type = 0;
  result.arg1 = 0;
  result.arg2 = 0;

  courier_ready = 1;
  while (1) {
    int j = 0;
    // while (j++ < 100000) asm("NOP");
    // printf(2, "request: %d\n\r", request);
    if (request) {
      char temp[10];
      for (i = 0; i < 10; ++i) {
        printf(2, "\033[s\033[H\033[2K%d\033[u", i);
        temp[i] = Getc(1);
      }
      printf(2, "\033[s\033[H%d\033[u", 2);
      printf(2, "\033[s\n\r\n\r\n\r\033[2K");
      printf(2, "\n\r\033[u", sender_tid);
      Send(SENSOR_TID, &temp, sizeof(temp), &dummy, sizeof(dummy));
      printf(2, "\033[s\033[H%d\033[u", 3);
      printf(2, "\033[s\n\r\n\r\n\r\033[2K");
      for (i = 0; i < 10; ++i)
        printf(2, "%d ", temp[i]);
      printf(2, "\n\r\033[u", sender_tid);
      // printf(2, "return from send\n\r");
      request = 0;
    }
    printf(2, "\033[s\n\r\n\r\n\r\n\r\n\r\n\r\n\r\033[2K request: %d\n\r\033[u", request);
    Receive( &sender_tid, &req, sizeof(struct cr_request));
    printf(2, "\033[s\n\r\n\r\n\r\n\r\033[2Ksender is %d\n\r\033[u", sender_tid);
    printf(2, "\033[s\n\r\n\r\n\r\n\r\n\r\033[2K\n\r\033[u", sender_tid);

    switch(req.type) {
      case CR_SET_SPEED:
        printf(1, "%c%c", req.arg2, req.arg1);
        train_list[req.arg1] = req.arg2;
        break;

      case CR_REVERSE_WAIT:
        if (train_list[req.arg1] == 0) { // train is already reversing
        } else {
          // set train speed to 0
          printf(1, "%c%c", 0, req.arg1);
          cr_reverse_list.speed[cr_reverse_list.tail] = train_list[req.arg1];
          cr_reverse_list.train[cr_reverse_list.tail] = req.arg1;
          cr_reverse_list.time[cr_reverse_list.tail++] = 500;
          cr_reverse_list.tail %= 100;
          train_list[req.arg1] = 0;
        }
        break;
        
      case CR_REVERSE_SET:

        break;

      case CR_SWITCH:
        Putc(1, 32);
        break;

      case CR_QUIT:
        break;

      case CR_SENSOR_REQUEST:
        Putc(1, 128 + 5);
        request = 1;
        break;

      default:
        break;
    }
    Reply(sender_tid, &result, sizeof(struct cr_request));
    printf(2, "\033[s\n\r\n\r\n\r\n\r\n\r\033[2Kreplied to %d\n\r\033[u", sender_tid);
  }
}





