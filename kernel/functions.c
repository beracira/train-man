#include "td.h"
#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "nameserver.h"
#include "clockserver.h"
#include "common.h"
#include "irq.h"
#include "kernel.h"
#include "io.h"
#include "train_ui.h"

#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

unsigned int idle_ticks = 0;

void idle_task(void) {
  int * timerLoad = (int *)(TIMER1_BASE + LDR_OFFSET);
  int * volatile timerValue = (int *)(TIMER1_BASE + VAL_OFFSET);
  int * timerControl = (int *)(TIMER1_BASE + CRTL_OFFSET);

  // 508000 cycles/s = 50800 cycles/0.1s
  int cyclesPerTick = ONE_TICK / 40;

  // Load the timer
  // Disable first
  *timerControl = (*timerControl) ^ ENABLE_MASK;
  // Load initial value
  *timerLoad = cyclesPerTick;
  // Enable timer, periodic mode, 508 kHz clock
  *timerControl = ENABLE_MASK | MODE_MASK| CLKSEL_MASK ;
  volatile int i = 16000000;
  int j = 0;
  (void) i;
  (void) j;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  volatile int * temp = &(ks->num_tasks);
  idle_ticks = 0;
  unsigned int pre = cyclesPerTick;
  while (*temp != 5) {
    unsigned int cur = *((int *)(timerValue));
    if (cur > pre) ++idle_ticks;
    pre = cur;
  }
  idle_print();
  Exit();
}

void idle_print() {
  bwprintf(COM2, "\033[s\033[A\033[50Cidle usage: %d%%\n\r\033[u", idle_ticks * 100 / 40 / time_ticks);
}

void the_other_task_1(void){
  int my_tid = MyTid();
  bwprintf(COM2, "Task %d created at: %d\n\r", my_tid, Time());
  int i;
  for(i = 0; i < 20; ++i) {
    Delay(10);
    bwprintf(COM2, "Tid %d: delay time is %d number of delays is %d\n\r", my_tid, 10, i + 1);
    idle_print();
  }
  bwprintf(COM2, "Task %d exiting at: %u\n\r", my_tid, Time());
  Exit();
}
void the_other_task_2(void){
  int my_tid = MyTid();
  bwprintf(COM2, "Task %d created at: %d\n\r", my_tid, Time());
  int i;
  for(i = 0; i < 9; ++i) {
    Delay(23);
    bwprintf(COM2, "Tid %d: delay time is %d number of delays is %d\n\r", my_tid, 23, i + 1);
    idle_print();
  }
  bwprintf(COM2, "Task %d exiting at: %u\n\r", my_tid, Time());
  Exit();
}
void the_other_task_3(void){
  int my_tid = MyTid();
  bwprintf(COM2, "Task %d created at: %d\n\r", my_tid, Time());
  int i;
  for(i = 0; i < 6; ++i) {
    Delay(33);
    bwprintf(COM2, "Tid %d: delay time is %d number of delays is %d\n\r", my_tid, 33, i + 1);
    idle_print();
  }
  bwprintf(COM2, "Task %d exiting at: %u\n\r", my_tid, Time());
  Exit();
}
void the_other_task_4(void){
  int my_tid = MyTid();
  bwprintf(COM2, "Task %d created at: %d\n\r", my_tid, Time());
  int i;
  for(i = 0; i < 3; ++i) {
    Delay(71);
    bwprintf(COM2, "Tid %d: delay time is %d number of delays is %d\n\r", my_tid, 71, i + 1);
    idle_print();
  }
  bwprintf(COM2, "Task %d exiting at: %u\n\r", my_tid, Time());
  Exit();
}

void firsttask(void) {

  
  // K3 tasks
  Create(P_HIGH, nameserver);
  Create(P_HIGH, clockserver);
  Create(P_HIGH, IO_Server);
  Create(P_SUPER_HIGH, timer_notifier);
  Create(P_HIGH, UI_Server);
  Create(P_LOW, &idle_task);

  // Create(3, &the_other_task_1);
  // Create(4, &the_other_task_2);
  // Create(5, &the_other_task_3);
  // Create(6, &the_other_task_4);

  //*data = 'x';

  // irq_enable_uart1_transmit();

  //irq_enable_uart2_receive();

  // irq_enable_uart2_transmit();

  //bwprintf(COM2, "char c %c \r\n", c);

  // flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
  // data = (int *)( UART1_BASE + UART_DATA_OFFSET );

  // *data = c;

  // char str[] = "let's print a string!\n\r";
  // int i;
  // int len = strlen(str);
  // int k;
  // for (k = 0; k < 50; k++)
  //   for (i = 0; i < 24; ++i) {
  //     Putc(1, str[i]);
  //     Putc(2, str[i]);
  //     // int a = 0;
  //     // while (++a < 100000) asm("");  
  //   }
  int k;
  for (k = 0; k < 10; k++) {
    printf(2, "test printf %d: %d \n\r", k, 1);
    //printf(1, str);
  }

  // while(1) {
  //   char c = Getc(2);
  //   Putc(1, c);
  //   // Putc(2, c);
  //   // Putc(2,counter);
  //   bwprintf(COM2, "%d\n\r", counter);
  //   bwprintf(COM2, "%d\n\r", err);
  // }

  // while (c != 'q'){
  //   c = bwgetc(COM2);

  // int *flags, *data;

  //   flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
  //   data = (int *)( UART2_BASE + UART_DATA_OFFSET );

  // while( ( *flags & TXFF_MASK ) ) ;
  // *data = c;
  // }

  Exit();
}

