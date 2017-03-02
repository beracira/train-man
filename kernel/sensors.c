#include "user_syscall.h"
#include "sensors.h"
#include "io.h"
#include "td.h"
#include "courier.h"

#define SENSOR_ARRAY_SIZE 10

int SENSOR_TID = 0;

void get_sensor_data() {
  SENSOR_TID = MyTid();

  int sensors[10];
  int sensor_len = 0;
  char sensor_letter[SENSOR_ARRAY_SIZE];
  int sensor_digit[SENSOR_ARRAY_SIZE];
  int i;
  for (i = 0; i < SENSOR_ARRAY_SIZE; ++i) {
    sensor_letter[i] = 0;
  }
  int tail = 0;
  int sender_tid = 10;
  char dummy = 0;
  int first = 1;
  while (1 + 2 == 3) {
    struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
    td[SENSOR_TID].state = SENSOR_BLOCKED;
    Pass();

    i = 0;
    while (i++ < 50000) asm("NOP");
    i = 0;
    while (i < 10) {
      // int flags = *((int *) 0x808c001c);
      printf(2, "\033[s\033[1;25H%d\033[u", i);
      int c = Getc(1);
      sensors[i++] = c;
    }
    int k;
    for (k = 0; k < 5; ++k) {
      int high = sensors[k * 2];
      int low = sensors[k * 2 + 1];
      // if (high == 46) high = 0;
      int radix = 16;
      while (low) {
        if (low & 1) {
          sensor_letter[tail] = 'A' + k;
          sensor_digit[tail] = radix;
          tail += 1;
          tail %= SENSOR_ARRAY_SIZE;
        }
        low >>= 1;
        radix--;
      }
      radix = 8;
      while (high) {
        if (high & 1) {
          sensor_letter[tail] = 'A' + k;
          sensor_digit[tail] = radix;
          tail += 1;
          tail %= SENSOR_ARRAY_SIZE;
        }
        high >>= 1;
        radix--;
      }
    }
    printf(2, "\033[s\033[26;30H\033[K");

    int i = (tail - 1 + SENSOR_ARRAY_SIZE) % SENSOR_ARRAY_SIZE;
    do {
      if (sensor_letter[i] != 0)
        printf(2, "%c%d ", sensor_letter[i], sensor_digit[i]);
      i -= 1;
      i += SENSOR_ARRAY_SIZE;
      i %= SENSOR_ARRAY_SIZE;
    } while (i != (tail - 1 + SENSOR_ARRAY_SIZE) % SENSOR_ARRAY_SIZE);

    for (i = 0; i < 10; ++i) {
      printf(2, "%d ", sensors[i]);
    }
    printf(2, "\033[u");
    td[CR_TID].state = READY;
  }
}
