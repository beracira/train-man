#ifndef PATH_FINDING_H
#define PATH_FINDING_H

extern int target_sensor;


extern short train_velocity[5][15][80][80];
int find_path(int train_number, int origin, int dest);
void train_velocity_init();

#endif // PATH_FINDING_H
