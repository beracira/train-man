#ifndef IO_H
#define IO_H

#include "../io/include/bwio.h"

#define IO_BUFFER_SIZE 102400

#define TRAIN_SEND       1
#define TRAIN_RECEIVE    2
#define TERMINAL_SEND    3
#define TERMINAL_RECEIVE 4

#define IO_TID           4



extern struct IO_Buffer * train_send_ptr;
extern struct IO_Buffer * train_receive_ptr;
extern struct IO_Buffer * terminal_send_ptr;
extern struct IO_Buffer * terminal_receive_ptr;

extern int io_ready;

struct IO_Request{
  int tid;
  int type;
  char c;
};

struct IO_Buffer{
  char buffer[IO_BUFFER_SIZE];
  int head;
  int tail;
};

struct IO_Wait_List{
  int tid[IO_BUFFER_SIZE];
  int head;
  int tail;
};

void IO_init();

void Putc(int uart, char ch);

char Getc(int uart);

inline void buffer_add(int uart, char c);

inline char buffer_remove(int uart);

void IO_Server();

void remove_wait_list(int uart);

void ioformat( int channel, char *fmt, va_list va );

void printf( int channel, char *fmt, ... );

#endif /* IO_H */
