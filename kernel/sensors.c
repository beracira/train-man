#include "user_syscall.h"
#include "sensors.h"
#include "io.h"

#define SENSOR_ARRAY_SIZE 10

int SENSOR_TID = 0;

void get_sensor_data() {
  SENSOR_TID = MyTid();

  char sensors[10];
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
  while (1 + 2 == 3) {
    // printf(2, "before Receive\n\r");
    Receive(&sender_tid, &sensors, sizeof(sensors));
    // printf(2, "after Receive\n\r");
    // while (sensor_len < 10) {
    //   int c = Getc(1);
    //   printf(2, "get!\n\r");
    //   sensors[sensor_len++] = c;
    //   printf(2, "%d\n\r", sensor_len);
    // }
    int k;
    for (k = 0; k < 5; ++k) {
      int high = sensors[k * 2];
      int low = sensors[k * 2 + 1];
      int radix = 16;
      while (low) {
        if (low & 1) {
          // putchar_terminal('A' + k);
          sensor_letter[tail] = 'A' + k;
          // put_int(radix);
          sensor_digit[tail] = radix;
          tail += 1;
          tail %= SENSOR_ARRAY_SIZE;
          // putchar_terminal(' ');
        }
        low >>= 1;
        radix--;
      }
      radix = 8;
      while (high) {
        if (high & 1) {
          // putchar_terminal('A' + k);
          sensor_letter[tail] = 'A' + k;
          // put_int(radix);
          sensor_digit[tail] = radix;
          tail += 1;
          tail %= SENSOR_ARRAY_SIZE;
          // putchar_terminal(' ');
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

    printf(2, "\033[u");
    Reply(sender_tid, &dummy, sizeof(dummy));
  }
}
