#include "dijkstra.h"
#include "track.h"
#include "io.h"
#include "clockserver.h"
#include "path_finding.h"

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
	// Delay(10);
	volatile track_node * track = (track_node *) TRACK_ADDR;

	int dist[TRACK_MAX];
	int visited[TRACK_MAX];
	int prev[TRACK_MAX];

	int i;
	for (i = 0; i < TRACK_MAX; i++) {
		prev[i] = -1;
		dist[i] = 1000000;
		visited[i] = 0;
	}

	int start = origin;
	dist[start] = 0;
  	dist[track[start].reverse->index] = 0;
	// visited[start] = 1;
	// visited[track[start].reverse->index] = 1;

	int next = 0;

	while (visited[dest] != 1 && visited[track[dest].reverse->index] != 1 && next != -1) {
		next = -1;
		// printf(2, "current: %s \n\r", track[start].name);
		// Delay(10);

		visited[start] = 1;
		// visited[track[start].reverse->index] = 1;

		if (track[start].type == NODE_BRANCH) {
			// printf(2, "branch \n\r");
			int i_straight = track[start].edge[DIR_STRAIGHT].dest->index;
			if (!visited[i_straight]){
				int d_straight = track[start].edge[DIR_STRAIGHT].dist;
				int m_dist = min_dist(dist[i_straight], d_straight + dist[start]);
				dist[i_straight] = m_dist;
				dist[track[i_straight].reverse->index] = m_dist;
				prev[i_straight] = start;
				prev[track[i_straight].reverse->index] = start;
			}
			int i_curved = track[start].edge[DIR_CURVED].dest->index;
			if (!visited[i_curved]){
				int d_curved = track[start].edge[DIR_CURVED].dist;
				int m_dist = min_dist(dist[i_curved], d_curved + dist[start]);
				dist[i_curved] = m_dist;
				dist[track[i_curved].reverse->index] = m_dist;
				prev[i_curved] = start;
				prev[track[i_curved].reverse->index] = start;
			}
		} else if (track[start].type == NODE_MERGE) {
			int i = track[start].edge[DIR_AHEAD].dest->index;
			if (!visited[i]){
				int d = track[start].edge[DIR_AHEAD].dist;
				int min = min_dist(dist[i], d + dist[start]);
				dist[i] = min;
				dist[track[i].reverse->index] = min;
				prev[i] = start;
				prev[track[i].reverse->index] = start;
			}

			int start_rev = track[start].reverse->index;

			int i_straight = track[start_rev].edge[DIR_STRAIGHT].dest->index;
			if (!visited[i_straight]) {
				int d_straight = track[start_rev].edge[DIR_STRAIGHT].dist;
				int m_dist = min_dist(dist[i_straight], d_straight + dist[start]);
				dist[i_straight] = m_dist;
				dist[track[i_straight].reverse->index] = m_dist;
				prev[i_straight] = start;
				prev[track[i_straight].reverse->index] = start;
			} 

			int i_curved = track[start_rev].edge[DIR_CURVED].dest->index;
			if (!visited[i_curved]) {
				int d_curved = track[start_rev].edge[DIR_CURVED].dist;
				int m_dist = min_dist(dist[i_curved], d_curved + dist[start]);
				dist[i_curved] = m_dist;
				dist[track[i_curved].reverse->index] = m_dist;
				prev[i_curved] = start;
				prev[track[i_curved].reverse->index] = start;
			}

		} else if (track[start].type != NODE_EXIT) {
			int i = track[start].edge[DIR_AHEAD].dest->index;
			if (!visited[i]) {
				int d = track[start].edge[DIR_AHEAD].dist;
				// printf(2, " else start: %d, i: %d ", start, i);
				int min_d = min_dist(dist[i], d + dist[start]);
				dist[i] = min_d;
				dist[track[i].reverse->index] = min_d;
				prev[i] = start;
				prev[track[i].reverse->index] = start;
			}
		}

		int min = 100000;
		for (i = 0; i < TRACK_MAX; i++) {
			// printf(2, "for loop: i: %d, visited: %d, dist: %d, start: %d \n\r", i, visited[i], dist[i], start);
			if (visited[i] == 0 && dist[i] < min && i != start) {
				min = dist[i];
				next = i;
			}
		}

		start = next;

	}


		// printf(2, "dest: %d. final: %d dest: %d rev: %d", visited[dest], visited[track[dest].reverse->index], dest, track[dest].reverse->index);
		// printf(2, "\n\rend: ");
	int final_dest;
	if (visited[dest] == 0) {
		final_dest = track[dest].reverse->index;
		if (visited[final_dest] == 0) return -1;
	} else {
		final_dest = dest;
	}
	int end = final_dest;
	i = 0;
	int temp[TRACK_MAX];
	int temp_len = 0;
	while (end != origin && end != track[origin].reverse->index) {
		// printf(2, "end: %s, %d, prev: %s \n\r", track[end].name, end, track[prev[end]].name);
		temp[temp_len++] = end;
		end = prev[end];
	}

	temp[temp_len] = origin;

	int j = temp_len;
	for (i = 0; i <= temp_len; i++) {
		path[i] = temp[j--];
	}

	return temp_len + 1;
}

// path is an array of track nodes
// parse_track_z breaks down the path into smaller paths, stored in the array smaller_paths,
// where the train needs to reverse between each of the paths.
// returns number of smaller paths
int parse_track_z(int * path, int len, int train, int current_sensor, struct Path * smaller_paths) {

	volatile track_node * track = (track_node *) TRACK_ADDR;

	int i;
	int j = 0;
	int k = 0;
	int l = 0;
	// int cur_sensor = current_sensor;
	int dist = 0;
	track_node * prev_node;
	track_node * cur_node;
	for (i = 0; i < len; i++) {
		// printf(2, "i: %d \n\r", i);
		cur_node = (track_node *) &track[path[i]];

		// special case in the middle
		if (   ((track[path[i]].index == 122 || track[path[i]].index == 123) && (track[path[i+1]].index == 120 || track[path[i+1]].index == 121))
			|| ((track[path[i]].index == 120 || track[path[i]].index == 121) && (track[path[i+1]].index == 122 || track[path[i+1]].index == 123))
			|| ((track[path[i]].index == 116 || track[path[i]].index == 117) && (track[path[i+1]].index == 118 || track[path[i+1]].index == 119))
			|| ((track[path[i]].index == 119 || track[path[i]].index == 119) && (track[path[i+1]].index == 116 || track[path[i+1]].index == 117))) {

			smaller_paths[j].node[k] = cur_node->index;
			smaller_paths[j].len = k+1;
			printf(2, "special move train to %s for and reverse, j: %d k: %d \n\r", cur_node->name, j, k);
			j++;
			k = 0;

		} else if (cur_node->type == NODE_SENSOR){
			printf(2, "1 move train to %s, j: %d k: %d \n\r", cur_node->name, j, k);
			if (cur_node->reverse->index == current_sensor) {
				smaller_paths[j].node[k] = cur_node->index;
				k++;
			} else if (i == len - 1) { // last node
				smaller_paths[j].node[k] = cur_node->index;
				smaller_paths[j].len = k + 1;
				j++;
				k++;
			} else {
				smaller_paths[j].node[k] = cur_node->index;
				k++;
			}
		} else if (cur_node->type == NODE_MERGE) {
			if (cur_node->edge[DIR_AHEAD].dest->index == path[i-1] ||
				cur_node->edge[DIR_AHEAD].dest->index == track[path[i-1]].reverse->index) {
				// treat as branch
				track_node * reverse = cur_node->reverse;
				path[i] = reverse->index;

				if (reverse->edge[DIR_STRAIGHT].dest->index == path[i+1]) {
					printf(2, "2 move train to %s, j: %d k: %d \n\r", cur_node->name, j, k);
					smaller_paths[j].node[k] = reverse->index;
					k++;
				} else if (reverse->edge[DIR_CURVED].dest->index == path[i+1]) {
					printf(2, "3 move train to %s, j: %d k: %d \n\r", cur_node->name, j, k);
					smaller_paths[j].node[k] = reverse->index;
					k++;
				} else {
					// what the motherfucking hell happened
				}

			} else {
				track_node * reverse = cur_node->reverse;
				if (reverse->edge[DIR_STRAIGHT].dest->index == path[i+1] ) {
					smaller_paths[j].node[k] = cur_node->index;
					smaller_paths[j].len = k+1;
					printf(2, "4 move train to %s and reverse, j: %d k: %d \n\r", cur_node->name, j, k);
					j++;
					k = 0;
				} else if (reverse->edge[DIR_STRAIGHT].dest->index == track[path[i+1]].reverse->index ) {
					path[i+1] = track[path[i+1]].reverse->index;
					smaller_paths[j].node[k] = cur_node->index;
					smaller_paths[j].len = k+1;
					printf(2, "4 move train to %s and reverse, j: %d k: %d \n\r", cur_node->name, j, k);
					j++;
					k = 0;
				} else if (reverse->edge[DIR_CURVED].dest->index == path[i+1]) {
					smaller_paths[j].node[k] = cur_node->index;
					smaller_paths[j].len = k+1;
					printf(2, "5 move train to %s and reverse, j: %d k: %d \n\r", cur_node->name, j, k);
					j++;
					k = 0;
				}  else if (reverse->edge[DIR_CURVED].dest->index == track[path[i+1]].reverse->index) {
					path[i+1] = track[path[i+1]].reverse->index;
					smaller_paths[j].node[k] = cur_node->index;
					smaller_paths[j].len = k+1;
					printf(2, "5 move train to %s and reverse, j: %d k: %d \n\r", cur_node->name, j, k);
					j++;
					k = 0;
				}else {
					printf(2, "6 move train to %s, j: %d k: %d \n\r", cur_node->name, j, k);
					smaller_paths[j].node[k] = cur_node->index;
					k++;
				}
			} 
		}
	}
	return j;
}


