#include "io.h"
#include "../io/include/ts7200.h"
#include "../io/include/bwio.h"
#include "user_syscall.h"
#include "td.h"
#include "syscall.h"

struct IO_Buffer * train_send_ptr = 0;
struct IO_Buffer * train_receive_ptr = 0;

struct IO_Buffer * terminal_send_ptr = 0;
struct IO_Buffer * terminal_receive_ptr = 0;

struct IO_Wait_List * train_wait_list_ptr = 0;
struct IO_Wait_List * terminal_wait_list_ptr = 0;

int io_ready = 0;

void IO_init() {
  int *high, *low;
  high = (int *)( UART1_BASE + UART_LCRM_OFFSET );
  low = (int *)( UART1_BASE + UART_LCRL_OFFSET );
  *high = 0x0;
  *low = 0xBF;
  *(int *)( UART1_BASE + UART_LCRH_OFFSET ) = 0x68;

  volatile int *flags, *data;

  flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
  data = (int *)( UART1_BASE + UART_DATA_OFFSET );
    
  irq_enable_uart1_receive();

  bwsetfifo( COM2, OFF );
  io_ready = 0;
}

void Putc(int uart, char ch) {
  if (uart == 1) {
    buffer_add(TRAIN_SEND, ch);
  } else {
    buffer_add(TERMINAL_SEND, ch);
  }
}

char Getc(int uart) {
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  
  struct IO_Request input;
  struct IO_Request output;

  input.c = 254;
  input.tid = ks->tid;
  output.c = 'a';
  if (uart == 1) {
    input.type = TRAIN_RECEIVE;
  } else {
    input.type = TERMINAL_RECEIVE;
  }

  Send(IO_TID, &input, sizeof(input), &output, sizeof(output));
  int sender_tid = 0;
  char ch = 98, dummy;

  Receive( &sender_tid, &ch, sizeof(char));
  Reply(sender_tid, &dummy, sizeof(char));

  return ch;
}

inline void buffer_add(int uart, char c) {
  struct IO_Buffer * target = 0;
  volatile int * uart_int_enable = 0;
  volatile int mask = 0;

  switch (uart) {
    case TRAIN_SEND:
      target = train_send_ptr;
      uart_int_enable = (int *) (UART1_BASE + UART_CTLR_OFFSET);
      mask = TIEN_MASK;
      *uart_int_enable |= mask;
      break;
    case TRAIN_RECEIVE:
      target = train_receive_ptr;
      // uart_int_enable = (int *) (UART1_BASE + UART_CTLR_OFFSET);
      // mask = RIEN_MASK;
      break;
    case TERMINAL_SEND:
      target = terminal_send_ptr;
      uart_int_enable = (int *) (UART2_BASE + UART_CTLR_OFFSET);
      mask = TIEN_MASK;
      *uart_int_enable |= mask;
      break;
    case TERMINAL_RECEIVE:
      target = terminal_receive_ptr;
      // uart_int_enable = (int *) (UART2_BASE + UART_CTLR_OFFSET);
      // mask = RIEN_MASK;
      break;
  }


  target->buffer[target->tail++] = c;
  target->tail %= IO_BUFFER_SIZE;

  if (uart == TRAIN_RECEIVE) {
    if (train_wait_list_ptr->head != train_wait_list_ptr->tail) {
      remove_wait_list(1);
    }
  } else if (uart == TERMINAL_RECEIVE) {
    if (terminal_wait_list_ptr->head != terminal_wait_list_ptr->tail) {
      remove_wait_list(2);
    }
  }
}

inline void buffer_add_printf(int uart, char c) {
  struct IO_Buffer * target = 0;

  switch (uart) {
    case 1:
      target = train_send_ptr;
      break;
    case 2:
      target = terminal_send_ptr;
      break;
  }

  target->buffer[target->tail++] = c;
  target->tail %= IO_BUFFER_SIZE;
}

inline char buffer_remove(int uart) {
  struct IO_Buffer * target = 0;
  volatile int * uart_int_enable = 0;
  volatile int mask = 0;
  volatile int *data = 0;

  switch (uart) {
    case TRAIN_SEND:
      target = train_send_ptr;
      uart_int_enable = (int *) (UART1_BASE + UART_CTLR_OFFSET);
      data = (int *)( UART1_BASE + UART_DATA_OFFSET );
      mask = TIEN_MASK;
      break;
    case TRAIN_RECEIVE:
      target = train_receive_ptr;
      uart_int_enable = (int *) (UART1_BASE + UART_CTLR_OFFSET);
      data = (int *)( UART1_BASE + UART_DATA_OFFSET );
      mask = RIEN_MASK;
      break;
    case TERMINAL_SEND:
      target = terminal_send_ptr;
      uart_int_enable = (int *) (UART2_BASE + UART_CTLR_OFFSET);
      data = (int *)( UART2_BASE + UART_DATA_OFFSET );
      mask = TIEN_MASK;
      break;
    case TERMINAL_RECEIVE:
      target = terminal_receive_ptr;
      uart_int_enable = (int *) (UART2_BASE + UART_CTLR_OFFSET);
      data = (int *)( UART2_BASE + UART_DATA_OFFSET );
      mask = RIEN_MASK;
      break;
  }


  if (target->head == target->tail) {
    *uart_int_enable &= ~mask;
  } else {
    char retval = target->buffer[target->head++];
    target->head %= IO_BUFFER_SIZE;
    if (uart == TRAIN_SEND || uart == TERMINAL_SEND)
      *data = retval;
    return retval;
  }

  return 255;
}



// inline char buffer_remove(struct IO_Buffer * target) {
//   if (target->head == target->tail) return -1;
//   char retval = target->buffer[target->head++];
//   target->head %= IO_BUFFER_SIZE;
//   return retval;
// }


void insert_wait_list(int uart, int tid) {
  if (uart == 1) {
    train_wait_list_ptr->tid[train_wait_list_ptr->tail++] = tid;
    train_wait_list_ptr->tail %= IO_BUFFER_SIZE;
  } else {
    terminal_wait_list_ptr->tid[terminal_wait_list_ptr->tail++] = tid;
    terminal_wait_list_ptr->tail %= IO_BUFFER_SIZE;
  }
}

char * get_char_buffer_ptr = 0;
int get_char_cur = 0;

void remove_wait_list(int uart) {
  if (uart == 1) {
    int tid = train_wait_list_ptr->tid[train_wait_list_ptr->head++];
    train_wait_list_ptr->head %= IO_BUFFER_SIZE;
    char ch, dummy;
    ch = buffer_remove(TRAIN_RECEIVE);
    get_char_buffer_ptr[get_char_cur] = ch;
    kernel_kernel_Send(tid, &(get_char_buffer_ptr[get_char_cur]), sizeof(char), &dummy, sizeof(char));
    get_char_cur += 1;
    get_char_cur %= 10240;
  } else {
    int tid = terminal_wait_list_ptr->tid[terminal_wait_list_ptr->head++];
    terminal_wait_list_ptr->head %= IO_BUFFER_SIZE;
    char ch, dummy;
    ch = buffer_remove(TERMINAL_RECEIVE);
    get_char_buffer_ptr[get_char_cur] = ch;
    kernel_kernel_Send(tid, &(get_char_buffer_ptr[get_char_cur]), sizeof(char), &dummy, sizeof(char));
    get_char_cur += 1;
    get_char_cur %= 10240;
  }
}


void IO_Server() {
  // IO_init(); // done by kernel init

  struct IO_Buffer train_send;
  struct IO_Buffer train_receive;
  struct IO_Buffer terminal_send;
  struct IO_Buffer terminal_receive;
  struct IO_Wait_List train_wait_list;
  struct IO_Wait_List terminal_wait_list;
  char get_char_buffer[10240];

  train_send_ptr = &train_send;
  train_receive_ptr = &train_receive;
  terminal_send_ptr = &terminal_send;
  terminal_receive_ptr = &terminal_receive;
  get_char_buffer_ptr = get_char_buffer;

  train_wait_list_ptr = &train_wait_list;
  terminal_wait_list_ptr = &terminal_wait_list;

  int i;
  for (i = 0; i < IO_BUFFER_SIZE; ++i) {
    train_send_ptr->buffer[i] = 0;
    train_receive_ptr->buffer[i] = 0;
    terminal_send_ptr->buffer[i] = 0;
    terminal_receive_ptr->buffer[i] = 0;
    train_wait_list_ptr->tid[i] = 0;
    terminal_wait_list_ptr->tid[i] = 0;
  }

  train_send_ptr->head = 0;
  train_send_ptr->tail = 0;
  train_receive_ptr->head = 0;
  train_receive_ptr->tail = 0;
  terminal_send_ptr->head = 0;
  terminal_send_ptr->tail = 0;
  terminal_receive_ptr->head = 0;
  terminal_receive_ptr->tail = 0;
  train_wait_list_ptr->head = 0;
  train_wait_list_ptr->tail = 0;
  terminal_wait_list_ptr->head = 0;
  terminal_wait_list_ptr->tail = 0;
  get_char_cur = 0;

  io_ready = 1;

  int sender_tid = -1;
  struct IO_Request req;
  struct IO_Request result;

  while (1 + 1 == 2) {
    Receive( &sender_tid, &req, sizeof(struct IO_Request));

    switch(req.type) {
      case TRAIN_RECEIVE:
        insert_wait_list(1, req.tid);
        break;

      case TERMINAL_RECEIVE:
        insert_wait_list(2, req.tid);
        break;

      default:
        break;
    }

    Reply(sender_tid, &result, sizeof(struct IO_Request));
  }
}

// void IO_Notifier() {
//   volatile int flags;
//   bwprintf(COM2, "%x\n\r", *((int *) 0x808d001c));
//   flags = *((int *) 0x808c001c); // COM1 
//   if (flags == 2) {
//     // RIS 
//   } else if (flags == 4) {
//     buffer_remove(TRAIN_SEND);
//     break;
//   } else {
//     //ERROR
//   }

//   flags = *((int *) 0x808d001c); // COM2
//   if (flags == 2) {
//     c = *data2;
//     buffer_add(TERMINAL_RECEIVE, c);
//   } else if (flags == 4) {
//     buffer_remove(TERMINAL_SEND);
//     break;
//   } else {
//     //ERROR
//   }
// }



char ioc2x( char ch ) {
  if ( (ch <= 9) ) return '0' + ch;
  return 'a' + ch - 10;
}

int ioputx( int channel, char c ) {
  char chh, chl;

  chh = ioc2x( c / 16 );
  chl = ioc2x( c % 16 );
  buffer_add_printf( channel, c);
  buffer_add_printf( channel, chl );
  return 0;
}

int ioputr( int channel, unsigned int reg ) {
  int byte;
  char *ch = (char *) &reg;

  for( byte = 3; byte >= 0; byte-- ) ioputx( channel, ch[byte] );
  buffer_add_printf(channel, ' ');
  return 0;
}

int ioputstr( int channel, char *str ) {
  while( *str ) {
    buffer_add_printf(channel, *str ); 
    str++;
  }
  return 0;
}

void ioputw( int channel, int n, char fc, char *bf ) {
  char ch;
  char *p = bf;

  while( *p++ && n > 0 ) n--;
  while( n-- > 0 ) buffer_add_printf( channel, fc );
  while( ( ch = *bf++ ) ) buffer_add_printf( channel, ch );
}

int ioa2d( char ch ) {
  if( ch >= '0' && ch <= '9' ) return ch - '0';
  if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
  if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
  return -1;
}

char ioa2i( char ch, char **src, int base, int *nump ) {
  int num, digit;
  char *p;

  p = *src; num = 0;
  while( ( digit = ioa2d( ch ) ) >= 0 ) {
    if ( digit > base ) break;
    num = num*base + digit;
    ch = *p++;
  }
  *src = p; *nump = num;
  return ch;
}

void ioui2a( unsigned int num, unsigned int base, char *bf ) {
  int n = 0;
  int dgt;
  unsigned int d = 1;
  
  while( (num / d) >= base ) d *= base;
  while( d != 0 ) {
    dgt = num / d;
    num %= d;
    d /= base;
    if( n || dgt > 0 || d == 0 ) {
      *bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
      ++n;
    }
  }
  *bf = 0;
}

void ioi2a( int num, char *bf ) {
  if( num < 0 ) {
    num = -num;
    *bf++ = '-';
  }
  ioui2a( num, 10, bf );
}

void ioformat ( int channel, char *fmt, va_list va ) {
  char bf[12];
  char ch, lz;
  int w;

  
  while ( ( ch = *(fmt++) ) ) {
    if ( ch != '%' )
      buffer_add_printf( channel, ch );
    else {
      lz = 0; w = 0;
      ch = *(fmt++);
      switch ( ch ) {
      case '0':
        lz = 1; ch = *(fmt++);
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        ch = ioa2i( ch, &fmt, 10, &w );
        break;
      }
      switch( ch ) {
      case 0: return;
      case 'c':
        buffer_add_printf( channel, va_arg( va, char ) );
        break;
      case 's':
        ioputw( channel, w, 0, va_arg( va, char* ) );
        break;
      case 'u':
        ioui2a( va_arg( va, unsigned int ), 10, bf );
        ioputw( channel, w, lz, bf );
        break;
      case 'd':
        ioi2a( va_arg( va, int ), bf );
        ioputw( channel, w, lz, bf );
        break;
      case 'x':
        ioui2a( va_arg( va, unsigned int ), 16, bf );
        ioputw( channel, w, lz, bf );
        break;
      case '%':
        buffer_add_printf( channel, ch );
        break;
      }
    }
  }
}

void printf( int channel, char *fmt, ... ) {
  va_list va;

  va_start(va,fmt);
  ioformat( channel, fmt, va );
  va_end(va);

  volatile int * uart_int_enable = 0;
  volatile int mask = 0;
  uart_int_enable = (int *) (UART1_BASE + UART_CTLR_OFFSET);
  mask = TIEN_MASK;
  if (channel == 2)
    uart_int_enable = (int *) (UART2_BASE + UART_CTLR_OFFSET);
  *uart_int_enable |= mask;
}

