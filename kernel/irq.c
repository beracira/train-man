#include "irq.h"
#include "../io/include/ts7200.h"
#include "clockserver.h"
#include "user_syscall.h"

void irq_enable_timer(void) {
  volatile int * int_enable = (int *) (VIC2_BASE + VICxIntEnable);
  *int_enable = *int_enable | (1 << (51 - 32));
}

void irq_clear_timer(void) {
  volatile int * clear = (int *) (TIMER3_BASE + CLR_OFFSET);
  *clear = 1;
}

void irq_disable_timer(void) {
  volatile int * int_en_clean = (int *) (VIC2_BASE + VICxIntEnClear);
  *int_en_clean = *int_en_clean | (1 << (51 - 32));
}

void timer_notifier(void) {
  int my_tid = MyTid();
  await_event_list_ptr[TIMER_EVENT] = my_tid;
  struct clk_request req;
  struct clk_request result;
  req.tid = my_tid;
  int i = 0;

  while (1 + 1 == 2) {
    printf(2, "before send: %d\n\r", ++i);
    AwaitEvent(TIMER_EVENT);
    req.type = CLK_INC;
    req.ticks = 0;
    printf(2, "asdf: %d\n\r", ++i);
    Send(CLK_TID, &req, sizeof(struct clk_request), &result, 0);
    printf(2, "after send: %d\n\r", ++i);
  }
}

void irq_enable_uart1_receive(void) {
  volatile int * uart1_int_enable = (int *) (UART1_BASE + UART_CTLR_OFFSET);
  volatile int * uart2_int_enable = (int *) (UART2_BASE + UART_CTLR_OFFSET);
  *uart1_int_enable &= ~TIEN_MASK;
  *uart1_int_enable |= RIEN_MASK;
  *uart1_int_enable |= MSIEN_MASK;
  *uart2_int_enable &= ~TIEN_MASK;
  *uart2_int_enable |= RIEN_MASK;
  *uart2_int_enable |= MSIEN_MASK;

  volatile int * int_enable = (int *) (VIC1_BASE + VICxIntEnable);
  *int_enable = *int_enable | (1 << 23);
  *int_enable = *int_enable | (1 << 24);
  *int_enable = *int_enable | (1 << 25);
  *int_enable = *int_enable | (1 << 26);
  // int_enable = (int *) (VIC2_BASE + VICxIntEnable);
  // *int_enable = *int_enable | (1 << (52 - 32));
  // *int_enable = *int_enable | (1 << (54 - 32));
}

// void irq_enable_uart1_transmit(void) {
//   // volatile int * uart_int_enable = (int *) (UART1_BASE + UART_CTLR_OFFSET);
//   // *uart_int_enable ^= TIEN_MASK;
//   volatile int * int_enable = (int *) (VIC1_BASE + VICxIntEnable);
//   *int_enable = *int_enable | (1 << (UART1_IntTransmit));
// }

void irq_enable_uart2_receive(void) {
  // volatile int * uart_int_enable = (int *) (UART2_BASE + UART_CTLR_OFFSET);
  // *uart_int_enable ^= RIEN_MASK;
  volatile int * vic_int_enable = (int *) (VIC1_BASE + VICxIntEnable);
  *vic_int_enable = *vic_int_enable | (1 << (UART2_IntReceive));
}

void irq_enable_uart2_transmit(void) {
  volatile int * uart_int_enable = (int *) (UART2_BASE + UART_CTLR_OFFSET);
  *uart_int_enable ^= TIEN_MASK;
  volatile int * vic_int_enable = (int *) (VIC1_BASE + VICxIntEnable);
  *vic_int_enable = *vic_int_enable | (1 << (UART2_IntTransmit));
}

