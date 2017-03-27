#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "path_finding.h"
int dijkstra(int * path, int origin, int dest);


int parse_track_z(int * path, int len, int train, int current_sensor, struct Path * smaller_paths);

#endif /* DIJKSTRA_H */
