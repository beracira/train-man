#include "dijkstra.h"
#include "track.h"
#include "io.h"
#include "clockserver.h"

#define NODE_NONE   0
#define NODE_SENSOR 1
#define NODE_BRANCH 2
#define NODE_MERGE  3
#define NODE_ENTER  4
#define NODE_EXIT  5

int min_dist(int a, int b) {
  return (a < b) ? a : b;
}

// returns the length of the path, or -1 if no path
// path is a ptr to an int array
// assumption: path is length TRACK_MAX
int dijkstra(int * path, int origin, int dest) {
  printf(2, "in dji \n\r");
  Delay(10);
  volatile track_node * track = (track_node *) TRACK_ADDR;

  int dist[TRACK_MAX];
  int visited[TRACK_MAX];
  int prev[TRACK_MAX];

  int i;
  for (i = 0; i < TRACK_MAX; i++) {
    prev[i] = -1;
    dist[i] = 10000;
    visited[i] = 0;
  }

  int start = origin;
  dist[start] = 0;
  visited[start] = 1;

  int next = 0;

  while (visited[dest] != 1 && next != -1) {
    next = -1;

    if (track[start].type == NODE_BRANCH) {

      int i_straight = track[start].edge[DIR_STRAIGHT].dest->index;
      int d_straight = track[start].edge[DIR_STRAIGHT].dist;

      dist[i_straight] = min_dist(dist[i_straight], d_straight + dist[start]);
      prev[i_straight] = start;

      int i_curved = track[start].edge[DIR_CURVED].dest->index;
      int d_curved = track[start].edge[DIR_CURVED].dist;

      dist[i_curved] = min_dist(dist[i_curved], d_curved + dist[start]);
      prev[i_curved] = start;

    } else if (track[start].type != NODE_EXIT) {
      int i = track[start].edge[DIR_AHEAD].dest->index;
      int d = track[start].edge[DIR_AHEAD].dist;

      dist[i] = min_dist(dist[i], d + dist[start]);
      prev[i] = start;
    }

    int min = 10000;
    for (i = 0; i < TRACK_MAX; i++) {
      if (visited[i] == 0 && dist[i] < min && i != start) {
        min = dist[i];
        next = i;
      }
    }

    start = next;
    visited[start] = 1;
  }

  if (visited[dest] == 0) return -1;

  int end = dest;
  i = 0;
  int temp[TRACK_MAX];
  int temp_len = 0;
  while (end != origin) {
    temp[temp_len++] = end;
    end = prev[end];
  }
  temp[temp_len] = origin;

  int j = temp_len;
  for (i = 0; i <= temp_len; i++) {
    path[i] = temp[j--];
  }

  return dist[dest];
}








