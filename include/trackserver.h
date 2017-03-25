#ifndef TRACKSERVER_H
#define TRACKSERVER_H

extern int sections[32];

#define TRACK_NEW_PATH 1
#define TRACK_RESERVE_SECTION 2
// #define TRACK_

#define TRACK_NOT_RESERVED 10
#define TRACK_RESERVED 11
#define TRACK_PATH_FOUND 12
#define TRACK_PATH_NOT_FOUND 13

struct track_request {
  int type;
  int * path;
  int path_len;
  int prev_section;
  int next_section;
  int train;
  int origin;
  int dest;
};

void track_server();
int request_new_path(int origin, int dest, int * path, int train);
int reserve_section(int next_section, int prev_section, int train);
#endif /* TRACKSERVER_H */
