#ifndef TRACK_H
#define TRACK_H

#define TRACK_A 1
#define TRACK_B 2

#define TRACK_ADDR 0x01700000

#define NODE_NONE   0
#define NODE_SENSOR 1
#define NODE_BRANCH 2
#define NODE_MERGE  3
#define NODE_ENTER  4
#define NODE_EXIT  5

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

struct track_node;
typedef struct track_node track_node;
typedef struct track_edge track_edge;

struct track_edge {
  track_edge *reverse;
  track_node *src, *dest;
  int dist;             /* in millimetres */
};

struct track_node {
  const char *name;
  int type;
  int num;              /* sensor or switch number */
  int dir;
  track_node *forward;
  track_node *reverse;  /* same location, but opposite direction */
  track_edge edge[2];
};

#define TRACK_MAX 144

void init_tracka(track_node *track);
void init_trackb(track_node *track);

void init_data(int track);
void track_init_switch(track_node *track);
void track_set_switch(int sw, int dir);

#endif /* TRACK_H */
