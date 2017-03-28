#include "../io/include/ts7200.h"
#include "train_ui.h"
#include "user_syscall.h"
#include "courier.h"
#include "io.h"
#include "functions.h"
#include "kernel.h"
#include "sensors.h"
#include "clockserver.h"
#include "track.h"
#include "trackserver.h"

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

int track_row = 0;
int track_col = 0;
void track_init(int row_start, int col_start);
void track_print_switch(int switch_number, int direction);


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
  // if (train_send_ptr->head != train_send_ptr->tail) {
  //   int i = train_send_ptr->head;
  //   while (i != train_send_ptr->tail) {
  //     printf(2, "%d ", train_send_ptr->buffer[i]);
  //     i += 1;
  //     i %= IO_BUFFER_SIZE;
  //   }
  //   printf(2, "\n\r");    
  // }
  print_sections();
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

    track_print_switch(switch_number, direction);
  }


  return 0;
}



void UI_init() {
  printf(2, "\033[2J"); // clear
  printf(2, "\033[H"); // back to top left
  printf(2, "Hello world.\n\r" ); // hi!
  printf(2, "Uptime: N/A" );

  track_init(track_row, track_col);

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
    track_print_switch(i, switches[i] == 'S' ? 33 : 34);
    Delay(20); 
  }

  for (i = 0x99; i <= 0x9c; ++i) {
    flip_switch(i, switches[i] == 'S' ? 33 : 34);
    printf(2, "%d\t%c\n\r", i, switches[i]);
    track_print_switch(i, switches[i] == 'S' ? 33 : 34);
    Delay(20); 
  }

  printf(2, "Recently triggered sensors: \n\rLast command:\n\r$ ");
}



void track_init(int row_start, int col_start) {
  int row = row_start;
  int col = col_start;

  printf(2, "\033[s");

  printf(2, "\033[%d;%dH______________________________________________________", row, col); 
  printf(2, "\033[%d;%dH____________/12 /11                                   \\", row+1, col);
  printf(2, "\033[%d;%dH___________/4  /_______________________________________\\9", row+2, col);
  printf(2, "\033[%d;%dH              /14          13\\_         _/10            \\", row+3, col);
  printf(2, "\033[%d;%dH             /                 \\_     _/                 \\", row+4, col);
  printf(2, "\033[%d;%dH            /                    \\ | /                    \\", row+5, col);
  printf(2, "\033[%d;%dH           |            0x9C(156) \\|/ 0x9B(155)            |", row+6, col);
  printf(2, "\033[%d;%dH           |                       |                       |", row+7, col);
  printf(2, "\033[%d;%dH           |                       |                       |", row+8, col);
  printf(2, "\033[%d;%dH           |                       |                       |", row+9, col);
  printf(2, "\033[%d;%dH           |            0x99(153) /|\\ 0x9A(154)            |", row+10, col);
  printf(2, "\033[%d;%dH            \\                   _/ | \\_                   /", row+11, col);
  printf(2, "\033[%d;%dH             \\                _/       \\_                / ", row+12, col);
  printf(2, "\033[%d;%dH            15\\______________/___________\\______________/8", row+13, col);
  printf(2, "\033[%d;%dH____________   \\             16          17            /", row+14, col);
  printf(2, "\033[%d;%dH____________\\1  \\_____________________________________/", row+15, col);
  printf(2, "\033[%d;%dH_____________\\2       6\\__                  __/7", row+16, col);
  printf(2, "\033[%d;%dH______________\\___________\\________________/_________", row+17, col);
  printf(2, "\033[%d;%dH               3          18               5", row+18, col);
  
  printf(2, "\033[u");

  int i;
  for (i = 0; i < 80; i = i+2){
    track_print_sensors(i, 0);
  }
}


void UI_Server() {
  
  track_row = 35;
  track_col = 5;

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

void track_print_section(int section, int c) {
  int colour = 35; // magenta by default
  if (c) colour = c;
  printf(2, "\033[s\033[0;37m");

  switch(section) {
    case 1:
      printf(2, "\033[%d;%dH\033[%dm_", track_row+17, track_col+0, colour); 
      printf(2, "\033[%d;%dH\033[%dm________", track_row+17, track_col+3, colour); 
      break;
    case 2:
      printf(2, "\033[%d;%dH\033[%dm_", track_row+16, track_col+0, colour); 
      printf(2, "\033[%d;%dH\033[%dm_______", track_row+16, track_col+3, colour); 
      break;
    case 3:
      printf(2, "\033[%d;%dH\033[%dm_", track_row+15, track_col+0, colour); 
      printf(2, "\033[%d;%dH\033[%dm______", track_row+15, track_col+3, colour); 
      break;
    case 4:
      printf(2, "\033[%d;%dH\033[%dm__", track_row+14, track_col+0, colour); 
      printf(2, "\033[%d;%dH\033[%dm_________", track_row+14, track_col+3, colour); 
      printf(2, "\033[%d;%dH\033[%dm______", track_row+17, track_col+15, colour); 
      break;
    case 5:
      printf(2, "\033[%d;%dH\033[%dm__________", track_row+0, track_col+0, colour); 
      break;
    case 6:
      printf(2, "\033[%d;%dH\033[%dm_________", track_row+1, track_col+0, colour); 
      break;
    case 7:
      printf(2, "\033[%d;%dH\033[%dm________", track_row+2, track_col+0, colour); 
      break;
    case 8:
      printf(2, "\033[%d;%dH\033[%dm_", track_row+0, track_col+11, colour);
      printf(2, "\033[%d;%dH\033[%dm___", track_row+0, track_col+13, colour);
      printf(2, "\033[%d;%dH\033[%dm___", track_row+0, track_col+17, colour);
      printf(2, "\033[%d;%dH\033[%dm_", track_row+1, track_col+10, colour);
      printf(2, "\033[%d;%dH\033[%dm__", track_row+2, track_col+9, colour);
      break;
    case 9:
      printf(2, "\033[%d;%dH\033[%dm___", track_row+0, track_col+13, colour);
      printf(2, "\033[%d;%dH\033[%dm___", track_row+0, track_col+17, colour);
      printf(2, "\033[%d;%dH\033[%dm_____", track_row+2, track_col+17, colour);
      printf(2, "\033[%d;%dH\033[%dm/", track_row+3, track_col+14, colour);
      printf(2, "\033[%d;%dH\033[%dm/", track_row+4, track_col+13, colour);
      break;
    case 10:
      printf(2, "\033[%d;%dH\033[%dm|", track_row+6, track_col+11, colour);
      printf(2, "\033[%d;%dH\033[%dm|", track_row+7, track_col+11, colour);
      printf(2, "\033[%d;%dH\033[%dm|", track_row+8, track_col+11, colour);
      printf(2, "\033[%d;%dH\033[%dm|", track_row+9, track_col+11, colour);
      printf(2, "\033[%d;%dH\033[%dm|", track_row+10, track_col+11, colour);
      break;
    case 11:
      printf(2, "\033[%d;%dH\033[%dm\\", track_row+12, track_col+13, colour);
      printf(2, "\033[%d;%dH\033[%dm\\", track_row+13, track_col+14, colour);
      printf(2, "\033[%d;%dH\033[%dm______", track_row+13, track_col+16, colour);
      printf(2, "\033[%d;%dH\033[%dm\\___", track_row+15, track_col+16, colour);
      break;
    case 12:
      printf(2, "\033[%d;%dH\033[%dm______", track_row+13, track_col+23, colour);
      printf(2, "\033[%d;%dH\033[%dm_", track_row+13, track_col+31, colour);
      printf(2, "\033[%d;%dH\033[%dm_", track_row+12, track_col+30, colour);
      break;
    case 13:
      printf(2, "\033[%d;%dH\033[%dm_____", track_row+13, track_col+33, colour);
      break;
    case 14:
      printf(2, "\033[%d;%dH\033[%dm_", track_row+11, track_col+32, colour);
      break;
    case 15:
      printf(2, "\033[%d;%dH\033[%dm|", track_row+7, track_col+35, colour);
      printf(2, "\033[%d;%dH\033[%dm|", track_row+8, track_col+35, colour);
      printf(2, "\033[%d;%dH\033[%dm|", track_row+9, track_col+35, colour);
      break;
    case 16:
      printf(2, "\033[%d;%dH\033[%dm______", track_row+2, track_col+23, colour);
      printf(2, "\033[%d;%dH\033[%dm__", track_row+2, track_col+30, colour);
      printf(2, "\033[%d;%dH\033[%dm_", track_row+3, track_col+30, colour);
      break;
    case 17:
      printf(2, "\033[%d;%dH\033[%dm_____", track_row+2, track_col+33, colour);
      break;
    case 18:
      printf(2, "\033[%d;%dH\033[%dm_", track_row+4, track_col+32, colour);
      break;
    case 19:
      printf(2, "\033[%d;%dH\033[%dm_", track_row+4, track_col+38, colour);
      break;
    case 20:
      printf(2, "\033[%d;%dH\033[%dm_", track_row+11, track_col+38, colour);
      break;
    case 21:
      printf(2, "\033[%d;%dH\033[%dm__", track_row+2, track_col+39, colour);
      printf(2, "\033[%d;%dH\033[%dm__", track_row+2, track_col+42, colour);
      printf(2, "\033[%d;%dH\033[%dm_", track_row+3, track_col+40, colour);  
      break;
    case 22:
      printf(2, "\033[%d;%dH\033[%dm_______", track_row+2, track_col+45, colour);
      break;
    case 23:
      printf(2, "\033[%d;%dH\033[%dm\\", track_row+1, track_col+54, colour);
      printf(2, "\033[%d;%dH\033[%dm_", track_row+2, track_col+53, colour);
      printf(2, "\033[%d;%dH\033[%dm\\", track_row+3, track_col+56, colour);  
      printf(2, "\033[%d;%dH\033[%dm\\", track_row+4, track_col+57, colour);
      printf(2, "\033[%d;%dH\033[%dm\\", track_row+5, track_col+58, colour);
      printf(2, "\033[%d;%dH\033[%dm|", track_row+6, track_col+59, colour);
      printf(2, "\033[%d;%dH\033[%dm|", track_row+7, track_col+59, colour);
      printf(2, "\033[%d;%dH\033[%dm|", track_row+8, track_col+59, colour);
      printf(2, "\033[%d;%dH\033[%dm|", track_row+9, track_col+59, colour);
      printf(2, "\033[%d;%dH\033[%dm|", track_row+10, track_col+59, colour);
      printf(2, "\033[%d;%dH\033[%dm/", track_row+11, track_col+58, colour);  
      printf(2, "\033[%d;%dH\033[%dm/", track_row+12, track_col+57, colour); 
      printf(2, "\033[%d;%dH\033[%dm/", track_row+13, track_col+56, colour); 
      printf(2, "\033[%d;%dH\033[%dm__", track_row+13, track_col+53, colour); 
      printf(2, "\033[%d;%dH\033[%dm/", track_row+15, track_col+54, colour);
      break;
    case 24:
      printf(2, "\033[%d;%dH\033[%dm____", track_row+15, track_col+49, colour); 
      break;
    case 25:
      printf(2, "\033[%d;%dH\033[%dm_______", track_row+13, track_col+45, colour); 
      break;
    case 26:
      printf(2, "\033[%d;%dH\033[%dm_", track_row+12, track_col+40, colour); 
      printf(2, "\033[%d;%dH\033[%dm_", track_row+13, track_col+39, colour); 
      printf(2, "\033[%d;%dH\033[%dm__", track_row+13, track_col+42, colour); 
      break;
    case 27:
      printf(2, "\033[%d;%dH\033[%dm________", track_row+0, track_col+45, colour); 
      break;
    case 28:
      printf(2, "\033[%d;%dH\033[%dm_______________________", track_row+0, track_col+21, colour); 
      break;
    case 29:
      printf(2, "\033[%d;%dH\033[%dm____", track_row+17, track_col+49, colour); 
      break;
    case 30:
      printf(2, "\033[%d;%dH\033[%dm____________", track_row+15, track_col+29, colour); 
      break;
    case 31:
      printf(2, "\033[%d;%dH\033[%dm__", track_row+15, track_col+21, colour); 
      printf(2, "\033[%d;%dH\033[%dm____", track_row+15, track_col+24, colour); 
      printf(2, "\033[%d;%dH\033[%dm____", track_row+15, track_col+42, colour); 
      printf(2, "\033[%d;%dH\033[%dm_", track_row+15, track_col+47, colour); 
      printf(2, "\033[%d;%dH\033[%dm__", track_row+16, track_col+24, colour); 
      printf(2, "\033[%d;%dH\033[%dm__", track_row+16, track_col+44, colour); 
      printf(2, "\033[%d;%dH\033[%dm___", track_row+17, track_col+22, colour); 
      printf(2, "\033[%d;%dH\033[%dm________________", track_row+17, track_col+27, colour); 
      printf(2, "\033[%d;%dH\033[%dm___", track_row+17, track_col+45, colour); 
      break;
    default:
      break;
  }
  printf(2, "\033[0;37m\033[u");

}

// c is the text colour, put 0 to default to cyan
void track_print_sensors(int sensor, int c) {
  int colour = 36; // cyan by default
  if (c) colour = c;

  printf(2, "\033[s\033[0;37m");
  switch(sensor) {
    case 0:
    case 1:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+0, track_col+10, colour); 
      break;
    case 2:
    case 3:
      printf(2, "\033[%d;%dH\033[1;%dm/", track_row+5, track_col+12, colour); 
      break;
    case 4:
    case 5:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+17, track_col+11, colour);
      break;
    case 6:
    case 7:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+16, track_col+10, colour); 
      break;
    case 8:
    case 9:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+15, track_col+9, colour); 
      break;
    case 10:
    case 11:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+14, track_col+2, colour); 
      break;
    case 12:
    case 13:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+1, track_col+9, colour); 
      break;
    case 14:
    case 15:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+2, track_col+8, colour); 
      break;
    case 16:
    case 17:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+13, track_col+32, colour); 
      break;
    case 18:
    case 19:
      printf(2, "\033[%d;%dH\033[1;%dm/", track_row+12, track_col+31, colour); 
      break;
    case 20:
    case 21:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+2, track_col+32, colour); 
      break;
    case 22:
    case 23:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+15, track_col+2, colour); 
      break;
    case 24:
    case 25:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+17, track_col+2, colour); 
      break;
    case 26:
    case 27:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+16, track_col+2, colour); 
      break;
    case 28:
    case 29:
      printf(2, "\033[%d;%dH\033[1;%dm\\", track_row+11, track_col+37, colour); 
      break;
    case 30:
    case 31:
      printf(2, "\033[%d;%dH\033[1;%dm\\", track_row+11, track_col+12, colour); 
      break;
    case 32:
    case 33:
      printf(2, "\033[%d;%dH\033[1;%dm/", track_row+11, track_col+33, colour); 
      break;
    case 34:
    case 35:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+17, track_col+48, colour); 
      break;
    case 36:
    case 37:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+15, track_col+20, colour); 
      break;
    case 38:
    case 39:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+17, track_col+21, colour); 
      break;
    case 40:
    case 41:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+13, track_col+22, colour); 
      break;
    case 42:
    case 43:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+2, track_col+22, colour); 
      break;
    case 44:
    case 45:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+0, track_col+20, colour); 
      break;
    case 46:
    case 47:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+15, track_col+28, colour); 
      break;
    case 48:
    case 49:
      printf(2, "\033[%d;%dH\033[1;%dm/", track_row+5, track_col+37, colour); 
      break;
    case 50:
    case 51:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+2, track_col+38, colour); 
      break;
    case 52:
    case 53:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+2, track_col+52, colour); 
      break;
    case 54:
    case 55:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+0, track_col+53, colour); 
      break;
    case 56:
    case 57:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+15, track_col+53, colour); 
      break;
    case 58:
    case 59:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+15, track_col+41, colour); 
      break;
    case 60:
    case 61:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+13, track_col+38, colour); 
      break;
    case 62:
    case 63:
      printf(2, "\033[%d;%dH\033[1;%dm\\", track_row+12, track_col+39, colour); 
      break;
    case 64:
    case 65:
      printf(2, "\033[%d;%dH\033[1;%dm\\", track_row+5, track_col+33, colour); 
      break;
    case 66:
    case 67:
      printf(2, "\033[%d;%dH\033[1;%dm/", track_row+4, track_col+39, colour); 
      break;
    case 68:
    case 69:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+2, track_col+44, colour); 
      break;
    case 70:
    case 71:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+0, track_col+44, colour); 
      break;
    case 72:
    case 73:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+13, track_col+52, colour); 
      break;
    case 74:
    case 75:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+15, track_col+48, colour); 
      break;
    case 76:
    case 77:
      printf(2, "\033[%d;%dH\033[1;%dm_", track_row+13, track_col+44, colour); 
      break;
    case 78:
    case 79:
      printf(2, "\033[%d;%dH\033[1;%dm\\", track_row+4, track_col+31, colour); 
      break;

    default:
      break;
  }

  printf(2, "\033[0;37m\033[u");
}

void track_print_switch(int switch_number, int direction) {

  volatile track_node * track = (track_node *) TRACK_ADDR;

  printf(2, "\033[s\033[0;37m");
  int colour = 32;  
  switch(switch_number) {
    case 1:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[0;37m_\033[1;%dm\\", track_row+15, track_col+11, colour); 
      } else {
        printf(2, "\033[%d;%dH\033[1;%dm_\033[0;37m\\", track_row+15, track_col+11, colour);
      }
      break;
    case 2:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[0;37m_\033[1;%dm\\", track_row+16, track_col+12, colour); 
      } else {
        printf(2, "\033[%d;%dH\033[1;%dm_\033[0;37m\\", track_row+16, track_col+12, colour); 
      }
      break;
    case 3:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[0;37m_\033[1;%dm\\", track_row+17, track_col+13, colour); 
      } else {
        printf(2, "\033[%d;%dH\033[1;%dm_\033[0;37m\\", track_row+17, track_col+13, colour); 
      }
      break;
    case 4:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[0;37m_", track_row+1, track_col+11); 
        printf(2, "\033[%d;%dH\033[1;%dm/", track_row+2, track_col+11, colour); 
      } else {
        printf(2, "\033[%d;%dH\033[1;%dm_", track_row+1, track_col+11, colour);
        printf(2, "\033[%d;%dH\033[0;37m/", track_row+2, track_col+11);
      }
      break;
    case 5:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[0;37m/\033[1;%dm_", track_row+17, track_col+43, colour); 
      } else {
        printf(2, "\033[%d;%dH\033[1;%dm/\033[0;37m_", track_row+17, track_col+43, colour); 
      }
      break;
    case 6:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[1;%dm_", track_row+15, track_col+23, colour);
        printf(2, "\033[%d;%dH\033[0;37m\\", track_row+16, track_col+23);
      } else {
        printf(2, "\033[%d;%dH\033[0;37m_", track_row+15, track_col+23); 
        printf(2, "\033[%d;%dH\033[1;%dm\\", track_row+16, track_col+23, colour); 
      }
      break;
    case 7:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[1;%dm_", track_row+15, track_col+46, colour);
        printf(2, "\033[%d;%dH\033[0;37m/", track_row+16, track_col+46);
      } else {
        printf(2, "\033[%d;%dH\033[0;37m_", track_row+15, track_col+46); 
        printf(2, "\033[%d;%dH\033[1;%dm/", track_row+16, track_col+46, colour); 
      }
      break;
    case 8:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[1;%dm_", track_row+13, track_col+55, colour);
        printf(2, "\033[%d;%dH\033[0;37m/", track_row+14, track_col+55);
      } else {
        printf(2, "\033[%d;%dH\033[0;37m_", track_row+13, track_col+55); 
        printf(2, "\033[%d;%dH\033[1;%dm/", track_row+14, track_col+55, colour); 
      }
      break;
    case 9:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[0;37m_\033[1;%dm\\", track_row+2, track_col+54, colour); 
      } else {
        printf(2, "\033[%d;%dH\033[1;%dm_\033[0;37m\\", track_row+2, track_col+54, colour); 
      }
      break;
    case 10:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[1;%dm_", track_row+2, track_col+41, colour);
        printf(2, "\033[%d;%dH\033[0;37m/", track_row+3, track_col+41);
      } else {
        printf(2, "\033[%d;%dH\033[0;37m_", track_row+2, track_col+41); 
        printf(2, "\033[%d;%dH\033[1;%dm/", track_row+3, track_col+41, colour); 
      }
      break;
    case 11:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[1;%dm_", track_row+0, track_col+16, colour);
        printf(2, "\033[%d;%dH\033[0;37m/", track_row+1, track_col+16);
      } else {
        printf(2, "\033[%d;%dH\033[0;37m_", track_row+0, track_col+16); 
        printf(2, "\033[%d;%dH\033[1;%dm/", track_row+1, track_col+16, colour); 
      }
      break;
    case 12:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[1;%dm_", track_row+0, track_col+12, colour);
        printf(2, "\033[%d;%dH\033[0;37m/", track_row+1, track_col+12);
      } else {
        printf(2, "\033[%d;%dH\033[0;37m_", track_row+0, track_col+12); 
        printf(2, "\033[%d;%dH\033[1;%dm/", track_row+1, track_col+12, colour); 
      }
      break;
    case 13:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[1;%dm_", track_row+2, track_col+29, colour);
        printf(2, "\033[%d;%dH\033[0;37m\\", track_row+3, track_col+29);
      } else {
        printf(2, "\033[%d;%dH\033[0;37m_", track_row+2, track_col+29); 
        printf(2, "\033[%d;%dH\033[1;%dm\\", track_row+3, track_col+29, colour); 
      }
      break;
    case 14:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[1;%dm/\033[0;37m_", track_row+2, track_col+15, colour); 
      } else {
        printf(2, "\033[%d;%dH\033[0;37m/\033[1;%dm_", track_row+2, track_col+15, colour); 
      }
      break;
    case 15:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[0;37m_", track_row+13, track_col+15); 
        printf(2, "\033[%d;%dH\033[1;%dm\\", track_row+14, track_col+15, colour); 
      } else {
        printf(2, "\033[%d;%dH\033[1;%dm_", track_row+13, track_col+15, colour);
        printf(2, "\033[%d;%dH\033[0;37m\\", track_row+14, track_col+15);
      }
      break;
    case 16:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[0;37m/\033[1;%dm_", track_row+13, track_col+29, colour); 
      } else {
        printf(2, "\033[%d;%dH\033[1;%dm/\033[0;37m_", track_row+13, track_col+29, colour); 
      }
      break;
    case 17:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[1;%dm_\033[0;37m\\", track_row+13, track_col+40, colour); 
      } else {
        printf(2, "\033[%d;%dH\033[0;37m_\033[1;%dm\\", track_row+13, track_col+40, colour); 
      }
      break;
    case 18:
      if (direction == 33) { // straight
        printf(2, "\033[%d;%dH\033[1;%dm_\033[0;37m\\", track_row+17, track_col+25, colour); 
      } else {
        printf(2, "\033[%d;%dH\033[0;37m_\033[1;%dm\\", track_row+17, track_col+25, colour); 
      }
      break;

    case 0x99:
      if (direction == 33) {
        if (track[118].dir == DIR_STRAIGHT) { // straight
          printf(2, "\033[%d;%dH\033[0;37m/\033[1;%dm|\033[0;37m\\", track_row+10, track_col+34, colour); 
        } else { // curve right (to sw 154)
          printf(2, "\033[%d;%dH\033[0;37m/|\033[1;%dm\\", track_row+10, track_col+34, colour); 
        }
      } else { 
        if (track[118].dir == DIR_STRAIGHT) { // curve left (to sw 153)
          printf(2, "\033[%d;%dH\033[1;%dm/\033[0;37m|\\", track_row+10, track_col+34, colour); 
        } else { 
          // get fucked
          printf(2, "\033[%d;%dH\033[1;5;31m/\033[0;37m|\033[1;5;31m\\", track_row+10, track_col+34, colour); 
        }
      }
      break;
    case 0x9a:
      if (direction == 33) {
        if (track[116].dir == DIR_STRAIGHT) { // straight
          printf(2, "\033[%d;%dH\033[0;37m/\033[1;%dm|\033[0;37m\\", track_row+10, track_col+34, colour); 
        } else {// curve left (to sw 153)
          printf(2, "\033[%d;%dH\033[1;%dm/\033[0;37m|\\", track_row+10, track_col+34, colour);
        }
      } else { 
        if (track[116].dir == DIR_STRAIGHT) {   // curve right (to sw 154)
          printf(2, "\033[%d;%dH\033[0;37m/|\033[1;%dm\\", track_row+10, track_col+34, colour); 
        } else { 
          // get fucked
          printf(2, "\033[%d;%dH\033[1;5;31m/\033[0;37m|\033[1;5;31m\\", track_row+10, track_col+34, colour); 
        }
      }
      break;
    case 0x9b:
      if (direction == 33) {
        if (track[122].dir == DIR_STRAIGHT) { // straight
          printf(2, "\033[%d;%dH\033[0;37m\\\033[1;%dm|\033[0;37m/", track_row+6, track_col+34, colour); 
        } else { // curve left (to sw 156)
          printf(2, "\033[%d;%dH\033[1;%dm\\\033[0;37m|/", track_row+6, track_col+34, colour);
        }
      } else { 
        if (track[122].dir == DIR_STRAIGHT) {   // curve right (to sw 155)
          printf(2, "\033[%d;%dH\033[0;37m\\|\033[1;%dm/", track_row+6, track_col+34, colour); 
        } else { 
          // get fucked
          printf(2, "\033[%d;%dH\033[1;5;31m\\\033[0;37m|\033[1;5;31m/", track_row+6, track_col+34, colour); 
        }
      }
      break;
    case 0x9c:
      if (direction == 33) {
        if (track[120].dir == DIR_STRAIGHT) { // straight
          printf(2, "\033[%d;%dH\033[0;37m\\\033[1;%dm|\033[0;37m/", track_row+6, track_col+34, colour); 
        } else { // curve right (to sw 154)
          printf(2, "\033[%d;%dH\033[0;37m\\|\033[1;%dm/", track_row+6, track_col+34, colour); 
        }
      } else { 
        if (track[120].dir == DIR_STRAIGHT) { // curve left (to sw 153)
          printf(2, "\033[%d;%dH\033[1;%dm\\\033[0;37m|/", track_row+6, track_col+34, colour); 
        } else { 
          // get fucked
          printf(2, "\033[%d;%dH\033[1;5;31m\\\033[0;37m|\033[1;5;31m/", track_row+6, track_col+34, colour); 
        }
      }
      break;
    default:
      break;
  }
  printf(2, "\033[0;37m\033[u");

}


