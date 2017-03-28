#ifndef DIJKSTRA_H
#define DIJKSTRA_H


struct zPath{
  int node[50];
  int len;
  int dist;
};

// #include "path_finding.h"
int dijkstra(int * path, int origin, int dest, int train);


int parse_track_z(int * path, int len, int train, int current_sensor, struct zPath * smaller_paths);


#endif /* DIJKSTRA_H */
