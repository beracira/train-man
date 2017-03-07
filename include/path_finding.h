#ifndef PATH_FINDING_H
#define PATH_FINDING_H


#define train_76 0
#define train_71 1
#define train_64 2

extern int target_sensor;

extern short train_velocity[5][15][80][80];
extern double default_speed[5][15];
int find_path(int train_number, int origin, int dest);
void train_velocity_init();

#endif // PATH_FINDING_H
