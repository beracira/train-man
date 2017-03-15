#include "../io/include/ts7200.h"
#include "train_ui.h"
#include "user_syscall.h"
#include "courier.h"
#include "io.h"
#include "functions.h"
#include "kernel.h"
#include "sensors.h"
#include "clockserver.h"

int UI_TID = 0;
int ui_ready = 0;

#define TENTH_SEC 50800
#define TIMER3_CONTROL 0x80810088
#define STRAIGHT 33
#define CURVE 34
#define DELAY(time) for (x = 0; x < (time);) { ++x; }
int setup = 0;
volatile int x = 0;
char cmd[1024] = {};
int speeds[100] = {};
char sw_state[0x9d] = {};
char output[4096] = {};
unsigned int output_head = 0;
unsigned int output_tail = 0;
char control_output[4096] = {};
unsigned int control_head = 0;
unsigned int control_tail = 0;

int com1_init() {
  int *high, *low;
  high = (int *)( UART1_BASE + UART_LCRM_OFFSET );
  low = (int *)( UART1_BASE + UART_LCRL_OFFSET );
  * high = 0x0;
  * low = 0xbf;
  * (int *) ( UART1_BASE + UART_LCRH_OFFSET) = 0x68;
  return 0;
}


int com2_init() {
  int *line, buf;
  line = (int *)( UART2_BASE + UART_LCRH_OFFSET );
  buf = *line;
  buf = buf & ~FEN_MASK;
  *line = buf;
  return 0;
}

void update_time() {
  unsigned int time = time_ticks / 10;
  unsigned int idle_percent = idle_ticks * 100 / 40 / time_ticks;
  idle_percent = idle_percent < 100 ? idle_percent : 100;

  printf(2, "\033[s\033[2;9H");
  int h = time / 10 / 60 / 60;
  if (h < 10) Putc(2, '0');
  printf(2, "%d:", h);
  int m = (time / 10 / 60) % 60;
  if (m < 10) Putc(2, '0');
  printf(2, "%d:", m);
  int s = (time / 10) % 60;
  if (s < 10) Putc(2, '0');
  printf(2, "%d.%d  Idle usage: %u%%\033[K", s, time % 10, idle_percent);
  printf(2, "\033[5;60H\033[KTime to next sensor: %d.%ds", time_to_next_sensor / 10, time_to_next_sensor % 10);
  printf(2, "\033[u");
}

int update_switch(int switch_number, int direction) {
  if ((switch_number >= 1 && switch_number <= 18) || (switch_number >= 0x99 && switch_number <= 0x9c)) {
    // int direction = cmd[i] == 'S' ? 33 : 34;
    // putchar_control(direction);
    // putchar_control(switch_number);
    int line_number = switch_number <= 18 ? 4 + switch_number : 4 + 18 + switch_number - 0x99 + 1;
    // put_string("\033[s");
    // put_string("\033[");
    // put_int(switch_number <= 17 ? 4 + switch_number : 4 + 17 + switch_number - 0x99 + 1);
    // put_string(";9H");
    // // putchar_terminal(direction);
    // put_string("\033[u");
    printf(2, "\033[s\033[%d;9H%c\033[u", line_number, direction == 33 ? 'S' : 'C');
  }
  return 0;
}

void UI_init() {
  printf(2, "\033[2J"); // clear
  printf(2, "\033[H"); // back to top left
  printf(2, "Hello world.\n\r" ); // hi!
  printf(2, "Uptime: N/A" );
  printf(2, "\n\rSwitch\tState\n\r--------------\n\r");
  int i;

  char switches[200];
  switches[1] = 'C';
  switches[2] = 'C';
  switches[3] = 'C';
  switches[4] = 'C';
  switches[5] = 'C';
  switches[6] = 'C';
  switches[7] = 'C';
  switches[8] = 'C';
  switches[9] = 'C';
  switches[10] = 'S';
  switches[11] = 'C';
  switches[12] = 'C';
  switches[13] = 'S';
  switches[14] = 'C';
  switches[15] = 'C';
  switches[16] = 'S';
  switches[17] = 'S';
  switches[18] = 'C';

  switches[0x99] = 'S';
  switches[0x9a] = 'C';
  switches[0x9b] = 'S';
  switches[0x9c] = 'C';

  for (i = 1; i <= 18; ++i) {
    flip_switch(i, switches[i] == 'S' ? 33 : 34);
    printf(2, "%d\t%c\n\r", i, switches[i]);
    Delay(20); 
  }

  for (i = 0x99; i <= 0x9c; ++i) {
    flip_switch(i, switches[i] == 'S' ? 33 : 34);
    printf(2, "%d\t%c\n\r", i, switches[i]);
    Delay(20); 
  }

  printf(2, "Recently triggered sensors: \n\rLast command:\n\r$ ");
}

void UI_Server() {
  
  UI_init();

  int sender_tid = -1;
  struct UI_Request req;
  struct UI_Request result;

  UI_TID = MyTid();
  ui_ready = 1;

  while (1 + 1 == 2) {
    Receive(&sender_tid, &req, sizeof(req));

    switch(req.type) {

      case UPDATE_SWITCH:
        update_switch(req.data1, req.data2);
        break;
      // case UPDATE_SENSORS:

      //   break;

      default:
        break;
    }
    Reply(sender_tid, &result, sizeof(result));
  }
}
