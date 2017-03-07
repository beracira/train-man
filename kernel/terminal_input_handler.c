#include "terminal_input_handler.h"
#include "courier.h"
#include "io.h"
#include "user_syscall.h"
#include "path_finding.h"
#include "stop.h"
#include "sensors.h"
#include "common.h"

int INPUT_TID = 0;

int stoi(char * str, int len) {
  int i;
  int retval = 0;
  for (i = 0; i < len; ++i) {
    retval += str[i] - '0';
    retval *= 10;
  }
  return retval / 10;
}

int command_parser(char * cmd, int cmd_len) {
  char * item[1024];
  char item_len[1024];
  int num_item = 0;
  while (*cmd == ' ') {
    ++cmd;
    --cmd_len;
  }
  int i;
  for (i = 0; i < 1024; ++i) item_len[i] = 0;
  int start = 0;
  for (i = 0; i < cmd_len; ++i) {
    if (cmd[i] == 0) break;
    if (cmd[i] != ' ') {
      if (!start) item[num_item] = cmd + i;
      item_len[num_item]++;
      start = 1;
    } else {
      if (start) {
        ++num_item;
        start = 0;
      }
    }
  }
  if (start) ++num_item; 
  if (num_item > 10) {
    printf(2, "\033[A\033[2K\rLast command: Invalid Command\033[B");
    return 1;
  } else {
    if (item_len[0] == 1) {
      if (item[0][0] == 'q') {
        printf(2, "\033[A\033[2K\rLast command: Quit\033[B");
        return -1;
      }
    } else if (item_len[0] <= 10) {
      item[0][(int)item_len[0]] = '\0';
      if (strcmp(item[0], "tr")) {
        int train_number = stoi(item[1], item_len[1]);
        int train_speed = stoi(item[2], item_len[2]);
        if (num_item != 3)
          printf(2, "\033[A\033[2K\rLast command: Invalid Command\033[B");
        else {
          set_train_speed(train_number, train_speed);
          printf(2, "\033[A\033[2K\rLast command: %s %d %d\033[B", item[0], train_number, train_speed);
        }
      } else if (strcmp(item[0], "rv")) {
        int train_number = stoi(item[1], item_len[1]);

        if (num_item != 2)
          printf(2, "\033[A\033[2K\rLast command: Invalid Command\033[B");
        else {
          reverse_train(train_number);
          printf(2, "\033[A\033[2K\rLast command: %s %d\033[B", item[0], train_number);
        }
      } else if (strcmp(item[0], "sw")) {
        int switch_number = stoi(item[1], item_len[1]);
        int direction = item[2][0] == 'S' || item[2][0] == 's' ? 33 : 34;
        if ((switch_number >= 1 && switch_number <= 18) || (switch_number >= 0x99 && switch_number <= 0x9c)) {


          if (num_item != 3) {
            printf(2, "\033[A\033[2K\rLast command: Invalid Command\033[B");
          } else {
            flip_switch(switch_number, direction);
            printf(2, "\033[A\033[2K\rLast command: %s %d %c\033[B", item[0], switch_number, item[2][0]);
          }
        } else if (strcmp(item[0], "stex")) {
          if (num_item >= 0) {
            printf(2, "\033[A\033[2K\rLast command: not implmented stex\033[B");
          } else {
            int train_number = stoi(item[1], item_len[1]);
            int exit_num = stoi(item[2], item_len[2]);
            if (exit_num > 10) {
              printf(2, "\033[A\033[2K\rLast command: stex no such exit\033[B");
              return 1;
            }
            // for (i = 0; i <)
            printf(2, "\033[A\033[2K\rLast command: %s %d %c\033[B", item[0], switch_number, item[2][0]);
          }
        } else {
          printf(2, "\033[A\033[2K\rLast command: Invalid Switch\033[B");
          return 1;
        }
      } else if (strcmp(item[0], "staf")) {
        int train_number = stoi(item[1], item_len[1]);
        int sensor = (item[2][0] - 'A') * 16 + stoi(item[2] + 1, item_len[2] - 1) - 1;
        item[2][item_len[2]] = 0;
        int delay = 0;
        if (num_item == 4) {
          delay = stoi(item[3], item_len[3]);
        }
        Stop(train_number, sensor, delay);
        printf(2, "\033[A\033[2K\rLast command: %s %d %s %d\033[B", item[0], train_number, item[2], delay);
      } else if (strcmp(item[0], "stat")) {
        int train_number = stoi(item[1], item_len[1]);
        int sensor = (item[2][0] - 'A') * 16 + stoi(item[2] + 1, item_len[2] - 1) - 1;
        int dist_init = 0;
        if (num_item == 4) {
          dist_init = stoi(item[3], item_len[3]);
        }
        int retval = find_path(train_number, last_sensor, sensor, dist_init);
        item[2][item_len[2]] = 0;
        if (retval == 0) {
          printf(2, "\033[A\033[2K\rLast command: %s %d %s %d\033[B", item[0], train_number, item[2], dist_init);
        } else {
          printf(2, "\033[A\033[2K\rLast command: cannot stop %d %s\033[B", train_number, item[2]);
        }
      } else if (strcmp(item[0], "cal")) {
        if (num_item != 3) {
          printf(2, "\033[A\033[2K\rLast command: not a good cal\033[B");
        }
        int train_number = stoi(item[1], item_len[1]);
        int speed = stoi(item[2], item_len[2]);
        printf(2, "\033[A\033[2K\rLast command: %s %d %d\033[B", item[0], train_number, speed);
        velocity_print(train_number, speed);
      }
    } else {
      printf(2, "\033[A\033[2K\rLast command: ERROR\033[B");
      return 1;
    }
  }
  return 0;
}

void input_handle() {
  
  INPUT_TID = MyTid();
  
  char cmd[1024];
  int cmd_len = 0;

  while (1 + 1 == 2) {
    char c = Getc(2);
    if (c == '\b') {
      if (cmd_len != 0) {
        cmd_len--;
        cmd[cmd_len] = 0;
        printf(2, "\b \b");
      }
    } else if (c != 13 && c != 11) {
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
         || c == ' ') {
        cmd[cmd_len++] = c;
        Putc(2, c);
      }
    } else {
      if (cmd_len != 0) {
        cmd[cmd_len++] = ' ';
        cmd[cmd_len++] = 0;
        int retval = command_parser(cmd, cmd_len);
        if (retval == -1) Exit();
      }
      cmd[cmd_len] = 0;
      int k = 0;
      for (k = 0; k < cmd_len; ++k) cmd[k] = 0;
      cmd_len = 0;
      printf(2, "\033[2K\r$ ");
    }
  }
  Exit();
}

