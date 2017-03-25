#include "trackserver.h"
#include "path_finding.h"
#include "user_syscall.h"


int sections[32] = {};

int TRACK_TID = 0;

void init_sections() {
  int i;
  for (i = 0; i < 32; i++) {
    sections[i] = 0;
  }
}

int request_new_path(int origin, int dest, int * path, int train){
  struct track_request input;
  struct track_request output;

  input.type = TRACK_NEW_PATH;
  input.path = path;
  input.path_len = 0;
  input.prev_section = 0;
  input.next_section = 0;
  input.train = train;
  input.origin = origin;
  input.dest = dest;

  output.type = 0;
  output.path = 0;
  output.path_len = 0;
  output.prev_section = 0;
  output.next_section = 0;
  output.train = 0;
  output.origin = -1;
  output.dest = -1;

  // double check tid
  Send(TRACK_TID, &input, sizeof(struct track_request), &output, sizeof(struct track_request));

  return output.path_len;
}

int reserve_section(int next_section, int prev_section, int train) {
  struct track_request input;
  struct track_request output;

  input.type = TRACK_RESERVE_SECTION;
  input.path = 0;
  input.path_len = 0;
  input.prev_section = prev_section;
  input.next_section = next_section;
  input.train = train;
  input.origin = -1;
  input.dest = -1;

  output.type = 0;
  output.path = 0;
  output.path_len = 0;
  output.prev_section = 0;
  output.next_section = 0;
  output.train = 0;
  output.origin = -1;
  output.dest = -1;

  // double check tid
  Send(TRACK_TID, &input, sizeof(struct track_request), &output, sizeof(struct track_request));

  return output.path_len;
}

void track_server() {
  init_sections();

  TRACK_TID = MyTid();

  int sender_tid = -1;
  struct track_request req;
  struct track_request result;
 
  while (1 + 1 == 2) {
    Receive( &sender_tid, &req, sizeof(struct track_request));

    if (req.type == TRACK_NEW_PATH) {
      // path finding
      int len = find_path_bfs(req.origin, req.dest, result.path, 0, 0);
      if (len) {
        result.type = TRACK_PATH_FOUND;
      } else {
        result.type = TRACK_PATH_NOT_FOUND;
      }

      result.path_len = len;
    }

    if (req.type == TRACK_RESERVE_SECTION) {
      if (sections[req.next_section] != 0) { // already reserved
        result.type = TRACK_NOT_RESERVED;
        result.train = sections[req.next_section];
      } else {
        sections[req.next_section] = req.train;
        sections[req.prev_section] = 0;
      }
    }

    Reply(sender_tid, &result, sizeof(struct track_request));
  }
}


void print_sections() {
  printf(2, "\033[s\033[18;40H\033[KUsed Section: ");
  int i;
  for (i = 1; i <= 31; ++i) {
    if (sections[i] != 0) {
      printf(2, "%d ", i);
    }
  }
  printf(2, "\033[u");
}
