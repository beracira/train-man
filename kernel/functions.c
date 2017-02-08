#include "td.h"
#include "functions.h"
#include "user_syscall.h"
#include "priorityqueue.h"
#include "nameserver.h"
#include "clockserver.h"
#include "common.h"
#include "irq.h"
#include "kernel.h"

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
  while (*temp != 4) {
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
  Create(P_SUPER_HIGH, timer_notifier);
  // Create(P_LOW, &idle_task);

  // Create(3, &the_other_task_1);
  // Create(4, &the_other_task_2);
  // Create(5, &the_other_task_3);
  // Create(6, &the_other_task_4);
  int *high, *low;
  high = (int *)( UART1_BASE + UART_LCRM_OFFSET );
  low = (int *)( UART1_BASE + UART_LCRL_OFFSET );
  *high = 0x0;
  *low = 0xBF;
  *(int *)( UART1_BASE + UART_LCRH_OFFSET ) = 0x68;

    volatile int *flags, *data;

    flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
    data = (int *)( UART1_BASE + UART_DATA_OFFSET );
    
  
  bwprintf(COM2, "first task started\n\r");
  irq_enable_uart1_receive();

  //*data = 'x';

  // irq_enable_uart1_transmit();

  //irq_enable_uart2_receive();

  // irq_enable_uart2_transmit();

  char c = bwgetc(COM2);

  //bwprintf(COM2, "char c %c \r\n", c);

      flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
    data = (int *)( UART1_BASE + UART_DATA_OFFSET );
    
  *data = c;

  while(1);
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

