#include "trackserver.h"
#include "path_finding.h"
#include "user_syscall.h"


int sections[32] = {};

void init_sections() {
  int i;
  for (i = 0; i < 32; i++) {
    sections[i] = 0;
  }
}




void track_server() {
  init_sections();

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
