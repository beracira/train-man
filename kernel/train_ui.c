#include "../io/include/ts7200.h"
#include "train_ui.h"
#include "user_syscall.h"
#include "courier.h"
#include "io.h"
#include "functions.h"
#include "kernel.h"
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
/* ---------------------------------- */
int putchar_terminal( char c ) {
  if (setup) {
    output[output_tail] = c;
    ++output_tail;
    output_tail &= 0xfff;
    return 0;
  } else {
    int *flags, *data;
    flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
    data = (int *)( UART2_BASE + UART_DATA_OFFSET );
    while( ( *flags & TXFF_MASK ) ) ;
    *data = c;
    return 0;
  }
}
int putchar_control( char c ) {
  if (setup) {
    control_output[control_tail] = c;
    ++control_tail;
    control_tail &= 0xfff;
    return 0;
  } else {
    int *flags, *data;
    flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
    data = (int *)( UART1_BASE + UART_DATA_OFFSET );
    while( ( *flags & TXFF_MASK ) ) ;
    *data = c;
    return 0;
 }
}
 char getchar_terminal() {
   volatile int *flags;
   volatile int *data;
   unsigned char c;
   flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
   data = (int *)( UART2_BASE + UART_DATA_OFFSET );
   while ( !( *flags & RXFF_MASK ) ) ;
   c = *data;
   return c;
 }
 char getchar_control() {
   int *flags, *data;
   unsigned char c;
   
   flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
   data = (int *)( UART1_BASE + UART_DATA_OFFSET );
   while ( !( *flags & RXFF_MASK ) ) ;
   c = *data;
   return c;
 }
 int put_string( char *str ) {
   while( *str ) {
     if( putchar_terminal( *str ) < 0 ) return -1;
     str++;
   }
   return 0;
 }
/* ---------------------------------- */
// int timer_init() {
//   *((int *) (TIMER3_BASE + LDR_OFFSET)) = (TENTH_SEC);
//   *((int *) (TIMER3_CONTROL)) =  0x000000c8;
//   return 0;
// }
int com1_init() {
  int *high, *low;
  high = (int *)( UART1_BASE + UART_LCRM_OFFSET );
  low = (int *)( UART1_BASE + UART_LCRL_OFFSET );
  * high = 0x0;
  * low = 0xbf;
  * (int *) ( UART1_BASE + UART_LCRH_OFFSET) = 0x68;
  return 0;
}

int put_int(int n) {
  if (n == 0) {
    putchar_terminal('0');
    return 0;
  }
  char temp[20];
  int len = 0;
  while (n) {
    temp[len++] = n % 10;
    n /= 10;
  }
  int i;
  for (i = len - 1; i >= 0; --i) {
    putchar_terminal('0' + temp[i]);
  }
  return 0;
}
int put_hex(int n) {
  if (n == 0) {
    put_string("0x0");
    return 0;
  }
  char temp[20];
  int len = 0;
  while (n) {
    temp[len++] = n % 16;
    n /= 16;
  }
  put_string("0x");
  int i;
  for (i = len - 1; i >= 0; --i) {
    if (temp[i] > 9) {
      putchar_terminal('a' + temp[i] - 10);
    } else {
      putchar_terminal('0' + temp[i]);
    }
  }
  return 0;
}
int put_time(int time) {
  int h = time / 10 / 60 / 60;
  if (h < 10) put_int(0);
  put_int(h);
  put_string(":");
  int m = (time / 10 / 60) % 60;
  if (m < 10) put_int(0);
  put_int(m);
  put_string(":");
  int s = (time / 10) % 60;
  if (s < 10) put_int(0);
  put_int(s);
  put_string(".");
  put_int(time % 10);
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


// int train_ui( int argc, char* argv[] ) {
//   int volatile x;
//   com1_init();
//   com2_init();
//   setup = 0;
//   printf(2, "\033[2J"); // clear
//   printf(2, "\033[H"); // back to top left
//   printf(2, "Hello world.\n\r" ); // hi!
//   printf(2, "Uptime: N/A" );
//   printf(2, "\n\rSwitch\tState\n\r--------------\n\r");
//   int counter = 0;
//   // char cmd[1024];
//   // int speeds[100];
//   // char sw_state[0x9d];
//   int i;
//   for (i = 0; i < 100; ++i) {
//     speeds[i] = 0;
//   }
//   for (i = 1; i <= 17; ++i) {
//     Putc(1, CURVE);
//     sw_state[i] = CURVE;
//     Putc(1, i);
//     printf(2, "%d\t%c\n\r", i, sw_state[i] == 33 ? 'S' : 'C');
//     DELAY(100000);
//     Putc(1, 32);
//     DELAY(100000);
//   }
  // for (i = 0x99; i <= 0x9c; ++i) {
  //   Putc(1, i & 1 ? STRAIGHT : CURVE);
  //   sw_state[i] = i & 1 ? STRAIGHT : CURVE;
  //   Putc(1, i);
  //   printf(2, "%d\t%c\n\r", i, sw_state[i] == 33 ? 'S' : 'C');
  //   DELAY(100000);
  //   Putc(1, 32);
  //   DELAY(100000);
  // }
//   printf(2, "Recently triggered sensors: \n\r$ ");
//   int cmd_len = 0;
//   int delay = 0;
//   int delay_second = 0;
//   int pre_time = 0;
//   int reversed_train_num;

//   volatile int *flags_1;
//   volatile int *flags_2;
//   volatile int *time_ptr;

//   flags_1 = (int *)( UART1_BASE + UART_FLAG_OFFSET );
//   flags_2 = (int *)( UART2_BASE + UART_FLAG_OFFSET );
//   time_ptr = ((int *) (TIMER3_BASE + VAL_OFFSET));

//   timer_init();
//   // polling loop
//   unsigned int pre = *time_ptr;
//   setup = 1;
//   output_head = 0;
//   output_tail = 0;
//   control_head = 0;
//   control_tail = 0;
//   for (i = 0; i < 4096; ++i) {
//     output[i] = 0;
//     control_output[i] = 0;
//   }
//   int sensor_requested = 0;
//   int sensors[10];
//   int sensor_len = 0;
//   int exit = 0;
//   while(1) {
//     if (exit && (control_head == control_tail)) return 0; 
//     unsigned int cur = *time_ptr;
//     if (pre < cur) {
//       put_string("\033[s");
//       put_string("\033[2;9H");
//       put_time(++counter);
//       put_string("\033[u");
//       *((int *) (0x8081000C)) = 1;
//       if (counter % 3 == 0) {
//         putchar_control(128 + 5);
//         sensor_requested = 1;
//       }
//     }
//     pre = cur;
//     if (sensor_requested) {
//       if ( *flags_1 & RXFF_MASK ) {
//         sensors[sensor_len++] = * (int *) (UART1_BASE + UART_DATA_OFFSET);
//       }
//       if (sensor_len == 10) {
//         put_string("\033[s");
//         put_string("\033[26;30H");  
//         put_string("\033[K");
//         int k;
//         for (k = 0; k < 5; ++k) {
//           int high = sensors[k * 2];
//           int low = sensors[k * 2 + 1];
//           int radix = 16;
//           while (low) {
//             if (low & 1) {
//               putchar_terminal('A' + k);
//               put_int(radix);
//               putchar_terminal(' ');
//             }
//             low >>= 1;
//             radix--;
//           }
//           radix = 8;
//           while (high) {
//             if (high & 1) {
//               putchar_terminal('A' + k);
//               put_int(radix);
//               putchar_terminal(' ');
//             }
//             high >>= 1;
//             radix--;
//           }
//         }
//         put_string("\033[u");
//         sensor_requested = 0;
//         sensor_len = 0;
//       }
//     }
//     cur = *time_ptr;
//     if (pre < cur) {
//       put_string("\033[s");
//       put_string("\033[2;9H");
//       put_time(++counter);
//       put_string("\033[u");
//       *((int *) (0x8081000C)) = 1;
//       if (counter % 3 == 0) {
//         putchar_control(128 + 5);
//         sensor_requested = 1;
//       }
//     }
//     pre = cur;
//     if ( *flags_1 & TXFE_MASK) {
//       if (control_head != control_tail) {
//         int *data;
//         data = (int *)( UART1_BASE + UART_DATA_OFFSET );
//         *data = control_output[control_head];
//         ++control_head;
//         control_head &= 0xfff;
//       }
//     }
//     if ( *flags_2 & TXFE_MASK) {
//       if (output_head != output_tail) {
//         int *data;
//         data = (int *)( UART2_BASE + UART_DATA_OFFSET );
//         *data = output[output_head];
//         ++output_head;
//         output_head &= 0xfff;
//       }
//     }
//     cur = *time_ptr;
//     if (pre < cur) {
//       put_string("\033[s");
//       put_string("\033[2;9H");
//       put_time(++counter);
//       put_string("\033[u");
//       *((int *) (0x8081000C)) = 1;
//       if (counter % 3 == 0) {
//         putchar_control(128 + 5);
//         sensor_requested = 1;
//       }
//     }
//     pre = cur;
//     if ( *flags_2 & RXFF_MASK) {
//       int *data;
//       data = (int *)( UART2_BASE + UART_DATA_OFFSET );
//       unsigned char c;
//       c = *data;
//       putchar_terminal(c);
//       if (c != 13 && c != 11) {
//         cmd[cmd_len++] = c;
//       } else {
//         cmd[cmd_len] = 0;
//         cmd_len = 0;
//         if (cmd[0] == 'q') {
//           exit = 1;
//           int j;
//           for (j = 0; i < 100; ++j) {
//             if (speeds[j]) {
//               putchar_control(0);
//               putchar_control(j);
//             }
//           }
//         }
//         put_string("\033[2K$ ");
//         if (cmd[0] == 't') {
//           int i = 3;
//           int len = 0;
//           for (;;) {
//             if (cmd[i++] == ' ') {
//               break;
//             }
//             ++len;
//           }
//           int train_number = stoi(cmd + 3, len);
//           len = 0;
//           int temp = i;
//           for (;;) {
//             if (cmd[i++] == 0) {
//               break;
//             }
//             ++len;
//           }
//           int train_speed = stoi(cmd + temp, len);
//           putchar_control(train_speed);
//           putchar_control(train_number);
//           speeds[train_number] = train_speed;
//         }
//         if (cmd[0] == 'r') {
//           pre_time = counter;
//           delay = 1;
//           int i = 3;
//           int len = 0;
//           for (;;) {
//             if (cmd[i++] == 0) {
//               break;
//             }
//             ++len;
//           }
//           reversed_train_num = stoi(cmd + 3, len);
//           putchar_control(0);
//           putchar_control(reversed_train_num);
//         }
//         if (cmd[0] == 's') {
//           int i = 3;
//           int len = 0;
//           for (;;) {
//             if (cmd[i++] == ' ') {
//               break;
//             }
//             ++len;
//           }
//           int switch_number = stoi(cmd + 3, len);
//           if ((switch_number >= 1 && switch_number <= 17) || (switch_number >= 0x99 && switch_number <= 0x9c)) {
//             int direction = cmd[i] == 'S' ? 33 : 34;
//             putchar_control(direction);
//             putchar_control(switch_number);
//             put_string("\033[s");
//             put_string("\033[");
//             put_int(switch_number <= 17 ? 4 + switch_number : 4 + 17 + switch_number - 0x99 + 1);
//             put_string(";9H");
//             putchar_terminal(cmd[i]);
//             put_string("\033[u");
//           }
//         }
//       }
//     }
//     cur = *time_ptr;
//     if (pre < cur) {
//       put_string("\033[s");
//       put_string("\033[2;9H");
//       put_time(++counter);
//       put_string("\033[u");
//       *((int *) (0x8081000C)) = 1;
//       if (counter % 3 == 0) {
//         putchar_control(128 + 5);
//         sensor_requested = 1;
//       }
//     }
//     pre = cur;
//     if (delay) {
//       if (counter - pre_time > 50) {
//         putchar_control(15);
//         putchar_control(reversed_train_num);
//         delay = 0;
//         delay_second = 1;
//         pre_time = counter;
//       }
//     }
//     if (delay_second) {
//       if (counter - pre_time > 20) {
//         putchar_control(speeds[reversed_train_num]);
//         putchar_control(reversed_train_num);
//         delay_second = 0;        
//       }
//     }
//     cur = *time_ptr;
//     if (pre < cur) {
//       put_string("\033[s");
//       put_string("\033[2;9H");
//       put_time(++counter);
//       put_string("\033[u");
//       *((int *) (0x8081000C)) = 1;
//     }
//     pre = cur;
//   }
//   return 0;
// }

void update_time() {
  unsigned int time = time_ticks / 10;
  unsigned int idle_percent = idle_ticks * 100 / 40 / time_ticks;
  idle_percent = idle_percent < 100 ? idle_percent : 100;

  printf(2, "\033[s\033[2;60H");
  int h = time / 10 / 60 / 60;
  if (h < 10) Putc(2, '0');
  printf(2, "%d:", h);
  int m = (time / 10 / 60) % 60;
  if (m < 10) Putc(2, '0');
  printf(2, "%d:", m);
  int s = (time / 10) % 60;
  if (s < 10) Putc(2, '0');
  printf(2, "%d.%d  Idle usage: %u%%\033[K\033[u", s, time % 10, idle_percent);
}

int update_switch(int switch_number, int direction) {
  if ((switch_number >= 1 && switch_number <= 17) || (switch_number >= 0x99 && switch_number <= 0x9c)) {
    // int direction = cmd[i] == 'S' ? 33 : 34;
    // putchar_control(direction);
    // putchar_control(switch_number);
    int line_number = switch_number <= 17 ? 4 + switch_number : 4 + 17 + switch_number - 0x99 + 1;
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

  for (i = 1; i <= 17; ++i) {
    if (i != 10 && i != 13 && i != 16 && i != 17) {
     flip_switch(i, 34);
     printf(2, "%d\t%c\n\r", i, 'C');
     Delay(20); 
    } else {
      flip_switch(i, 33);
      printf(2, "%d\t%c\n\r", i, 'S');
      Delay(20); 
    }
  }

  for (i = 0x99; i <= 0x9c; ++i) {
    flip_switch(i, i & 1 ? 33 : 34);
    printf(2, "%d\t%c\n\r", i, i & 1 ? 'S' : 'C');
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
