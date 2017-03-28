#include "courier.h"
#include "user_syscall.h"
#include "clockserver.h"
#include "io.h"
#include "train_ui.h"
#include "track.h"
#include "sensors.h"
#include "td.h"
#include "path_finding.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

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
  
  if (train == train_64_struct.train_number) {
    train_64_struct.speed = speed;
  } else if (train == officer_struct.train_number) {
    officer_struct.speed = speed;
  }

  Send(CR_TID, &input, sizeof(struct cr_request), &output, sizeof(struct cr_request));
}

void reverse_train(int train) {
  struct cr_request input;
  struct cr_request output;

  input.type = CR_REVERSE_WAIT;
  input.arg1 = train;
  input.arg2 = 0;

  if (train == train_64_struct.train_number) {
    train_64_struct.direction ^= 1;
  } else if (train == officer_struct.train_number) {
    officer_struct.direction ^= 1;
  }

  Send(CR_TID, &input, sizeof(struct cr_request), &output, sizeof(struct cr_request));
}

void wake_train() {
  int first_temp = 0;
  if (cr_reverse_list_ptr->time[cr_reverse_list_ptr->head] == 0) {
    // reverse train
    printf(1, "%c%c", 15, cr_reverse_list_ptr->train[cr_reverse_list_ptr->head]);
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

    Send(UI_TID, (struct UI_Request *) &input, sizeof(struct UI_Request), (struct UI_Request *) &output, sizeof(struct UI_Request));
  }

  input.type = CR_SWITCH;
  input.arg1 = switch_num;
  input.arg2 = dir;

  Send(CR_TID, &input, sizeof(struct cr_request), &output, sizeof(struct cr_request));

  if (dir == 33) {
    track_set_switch(switch_num, DIR_STRAIGHT);
  } else {
    track_set_switch(switch_num, DIR_CURVED);
  }
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
  req.type = -1;
  result.type = 0;
  result.arg1 = 0;
  result.arg2 = 0;

  courier_ready = 1;
  while (1) {
    // if (sensor_requested) {
    // }
    if (req.type != -1) {
      if (req.type == CR_SET_SPEED) {
        bwprintf(0, "%c%c", req.arg2, req.arg1);
        while (i++ < 50000) asm("");
      } else if (req.type == CR_REVERSE_WAIT && train_list[req.arg1] != 0) {
        bwprintf(0, "%c%c", 0, req.arg1);
        train_list[req.arg1] = 0;
        // Delay(3);
      } else if (req.type == CR_SWITCH) {
        bwprintf(0, "%c%c", req.arg2, req.arg1);
        // Delay(3);
        volatile int i;
        while (i++ < 50000) asm("");
        bwprintf(0, "%c", 32);
        // Putc(1, 32);
        // Delay(3);
      } else if (req.type == CR_SENSOR_REQUEST) {
        volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
        volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
        int start_time = time_ticks;
        td[SENSOR_TID].state = SENSOR_BLOCKED_2;
        bwprintf(0, "%c", 128 + 5);
        // Putc(1, 128 + 5);
        volatile int i;
        while (i++ < 80000) asm("");
        // Delay(3);
        while (sensor_len < 10) {
          volatile int *flags, *data;
          unsigned char c;
          flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
          data = (int *)( UART1_BASE + UART_DATA_OFFSET );
          if ( ( *flags & RXFF_MASK ) ) {
            c = *data;
            sensors[sensor_len++] = c;
          }
          // char c = bwgetc(0);
          if (time_ticks - start_time >= 10) break;
        }
        if (SENSOR_TID != -1 && td[SENSOR_TID].state == SENSOR_BLOCKED_2) td[SENSOR_TID].state = READY;
      }
      // printf(2, "%d \n\r", req.type);
    }
    volatile int i;
    while (i++ < 50000) asm("");
    Receive( &sender_tid, &req, sizeof(struct cr_request));

    switch(req.type) {
      case CR_SET_SPEED:
        // printf(1, "%c%c", req.arg2, req.arg1);
        train_list[req.arg1] = req.arg2;
        break;

      case CR_REVERSE_WAIT:
        if (train_list[req.arg1] == 0) { // train is already reversing
        } else {
          // set train speed to 0
          // printf(1, "%c%c", 0, req.arg1);
          cr_reverse_list.speed[cr_reverse_list.tail] = train_list[req.arg1];
          cr_reverse_list.train[cr_reverse_list.tail] = req.arg1;
          cr_reverse_list.time[cr_reverse_list.tail++] = 500;
          cr_reverse_list.tail %= 100;
          // train_list[req.arg1] = 0;
        }
        break;
        
      case CR_REVERSE_SET:

        break;

      case CR_SWITCH:
        // printf(1, "%c%c", req.arg2, req.arg1);
        // Putc(1, 32);
        break;

      case CR_QUIT:
        break;

      default:
        break;
    }
    Reply(sender_tid, &result, sizeof(struct cr_request));
  }
}





