#ifndef RPS_H
#define RPS_H

#define RPS_SIGN_UP 0
#define RPS_PLAY 1
#define RPS_QUIT 2
#define RPS_PLAY_RECEIVED 3
#define RPS_GET_RESULTS 4
#define RPS_ERROR = 5

#define PLAYER_QUIT 2
#define PLAYER_WIN 3
#define PLAYER_LOSE 4
#define PLAYER_TIE 5

#define RPS_ROCK 1
#define RPS_PAPER 2
#define RPS_SCISSORS 3

struct rps_request {
  int type;
  int move;
  int tid;
};

struct rps_server{
  int move;
  int tid;
};

int rps_sign_up(void);

int rps_play(int move);

int rps_quit(void);

void rps_server(void);

#endif /* RPS_H */
