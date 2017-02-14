#include "terminal_input_handler.h"
#include "courier.h"
#include "io.h"

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
  if (num_item > 3) {
    printf(2, "\033[A\033[KLast command: Invalid Command\033[B");
    return 1;
  } else {
    if (item_len[0] == 1) {
      if (item[0][0] == 'q') {
        printf(2, "\033[A\033[KLast command: Quit\033[B");
        return -1;
      }
    } else if (item_len[0] == 2) {
      item[0][2] = 0;
      if (item[0][0] == 't') {
        int train_number = stoi(item[1], item_len[1]);
        int train_speed = stoi(item[2], item_len[2]);
        if (num_item != 3)
          printf(2, "\033[A\033[KLast command: Invalid Command\033[B");
        else {
          // set_train_speed(train_number, train_speed);
          // printf(1, "%c%c", train_number, train_speed);
          printf(2, "\033[A\033[KLast command: %s %d %d\033[B", item[0], train_number, train_speed);
        }
      } else if (item[0][0] == 'r') {
        int train_number = stoi(item[1], item_len[1]);

        if (num_item != 3)
          printf(2, "\033[A\033[KLast command: Invalid Command\033[B");
        else {
          // reverse_train(train_number);
          printf(2, "\033[A\033[KLast command: %s %d\033[B", item[0], train_number);
        }
      } else if (item[0][0] == 's') {
        int switch_number = stoi(item[1], item_len[1]);
        if ((switch_number >= 1 && switch_number <= 17) || (switch_number >= 0x99 && switch_number <= 0x9c)) {


          if (num_item != 2)
            printf(2, "\033[A\033[KLast command: Invalid Command\033[B");
          else {
            // flip_switch(switch_number, 34);
            printf(2, "\033[A\033[KLast command: %s %d %d\033[B", item[0], switch_number);
          }
        } else {
          printf(2, "\033[A\033[KLast command: Invalid Switch\033[B");
          return 1;
        }
      }
    } else {
      printf(2, "\033[A\033[KLast command: ERROR\033[B");
      return 1;
    }
  }
}

void input_handle() {
  char cmd[1024];
  int cmd_len = 0;

  while (1 + 1 == 2) {
    char c = Getc(2);
    if (c == '\b') {
      if (cmd_len != 0) {
        cmd_len--;
        cmd[cmd_len] = 0;
        printf(2, "\033[s\n\r\n\r\033[2K%d\033[u", cmd_len);
        printf(2, "\b \b");
      }
    } else if (c != 13 && c != 11) {
      cmd[cmd_len++] = c;
      Putc(2, c);
    } else {
      if (cmd_len != 0) {
        cmd[cmd_len++] = ' ';
        cmd[cmd_len++] = 0;
        printf(2, "\033[s\n\r\033[2K%s\033[u", cmd);
        command_parser(cmd, cmd_len);
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

