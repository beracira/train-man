#include "terminal_input_handler.h"
#include "io.h"

void command_parser(char * cmd) {

}

void input_handle() {
  char * cmd[1024];
  int cmd_len = 0;

  while (1 + 1 == 2) {
    char c = Getc(2);
    Putc(c);
    if (c != 13 && c != 11) {
      cmd[cmd_len++] = c;
    } else {
      command_parser(cmd, cmd_len);
      cmd[cmd_len] = 0;
      cmd_len = 0;
      printf("\033[2K$ ");
    }
  }
}

