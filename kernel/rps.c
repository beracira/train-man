#include "rps.h"
#include "nameserver.h"
#include "common.h"
#include "user_syscall.h"
#include "td.h"
#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"

int rps_sign_up(void) {
  struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;

  struct rps_request input;
  struct rps_request output;

  input.type = RPS_SIGN_UP;
  input.tid = ks->tid;

  //bwprintf(COM2, "before signup send \n\r");
  Send(WhoIs("RPS"), &input, sizeof(struct rps_request), &output, sizeof(struct rps_request));
  //printf(COM2, "after signup send \n\r");
  // todo error checking 
  return 0;
}


int rps_play(int move) {
  struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;

  struct rps_request input;
  struct rps_request output;

  input.type = RPS_PLAY;
  input.tid = ks->tid;
  input.move = move;

  Send(WhoIs("RPS"), &input, sizeof(struct rps_request), &output, sizeof(struct rps_request));

  // todo error checking 
  return 0;
}

int rps_quit(void) {
  struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;

  struct rps_request input;
  struct rps_request output;

  input.type = RPS_QUIT;
  input.tid = ks->tid;

  Send(WhoIs("RPS"), &input, sizeof(struct rps_request), &output, sizeof(struct rps_request));

  // todo error checking 
  return 0;
}

void rps_server(void) {
    RegisterAs("RPS");
    bwprintf(COM2, "in rps\n\r");

    struct rps_server play_queue[50];
    int players[50];

    int i;
    for (i = 0; i < 50; ++i) {
      play_queue[i].tid = -1;
      play_queue[i].move = -1;
      players[i] = -1;
    }

    int first_play_queue = 0;
    int last_play_queue = 0;
    int num_play_queue = 0;

    int first_temp = 0;
    int last_temp = 0;

    while (1 + 1 == 2) {

      int sender_tid = 0;
      struct rps_request req;
      struct rps_request result;

      if (num_play_queue >= 2) {
        int p1_tid = play_queue[first_play_queue].tid;
        int p1_move = play_queue[first_play_queue].move;
        int p2_tid = play_queue[(first_play_queue+1)%50].tid;
        int p2_move = play_queue[(first_play_queue+1)%50].move;

        char * m1;
        char * m2;
        
        switch (p1_move){
          case RPS_ROCK:
            m1 = "Rock";
            break;
          case RPS_PAPER:
            m1 = "Paper";
            break;
          case RPS_SCISSORS:
            m1 = "Scissors";
            break;
        }

        switch (p2_move){
          case RPS_ROCK:
            m2 = "Rock";
            break;
          case RPS_PAPER:
            m2 = "Paper";
            break;
          case RPS_SCISSORS:
            m2 = "Scissors";
            break;
        }

        bwprintf(COM2, "TID %d plays %s and TID %d plays %s\n\r", p1_tid, m1, p2_tid, m2);

        if (p1_move == p2_move) {
          bwprintf(COM2, "Result: Tied \n\r");
        }

        else if ( (p1_move == RPS_ROCK && p2_move == RPS_SCISSORS) ||
                  (p1_move == RPS_PAPER && p2_move == RPS_ROCK) ||
                  (p1_move == RPS_SCISSORS && p2_move == RPS_PAPER) ) {
          bwprintf(COM2, "Result: TID %d wins!\n\r", p1_tid);
        }

        else {
          bwprintf(COM2, "Result: TID %d wins!\n\r", p2_tid);
        }

        play_queue[first_play_queue].tid = -1;
        play_queue[first_play_queue].move = -1;
        play_queue[(first_play_queue+1)%50].tid = -1;
        play_queue[(first_play_queue+1)%50].move = -1;

        first_play_queue = (first_play_queue + 2) % 50;

        players[p1_tid] = 0;
        players[p2_tid] = 0;
      }

      Receive( &sender_tid, &req, sizeof(struct rps_request));
      bwprintf(COM2, "after rps receive\n\r");

      switch(req.type) {
        case RPS_SIGN_UP:
          bwprintf(COM2, "rps sign up1\n\r");
          players[req.tid] = 0;
          bwprintf(COM2, "rps sign up2\n\r");
          break;

        case RPS_PLAY:

          switch(players[req.tid]) {
            case -1: // not signed up
              break;
            
            case 0: // hasn't played yet
              play_queue[last_play_queue].tid = req.tid;
              play_queue[last_play_queue].move = req.move;
              players[req.tid] = 1;
              last_play_queue = (last_play_queue + 1) % 50;
              num_play_queue++;
              break;
            
            case 1: // already played
              first_temp = first_play_queue;
              last_temp = last_play_queue;
              while (first_temp != last_temp) {
                if (play_queue[first_temp].tid == req.tid) {
                  play_queue[first_temp].move = req.move;
                  break;
                }
                first_temp = (first_temp + 1) % 50;
              }
              break;
            default:
              break;
          }
          
          break; // player hasn't signed up

        case RPS_QUIT:
          switch(players[req.tid]) {
            case -1: // not signed up
              break;

            case 0: // hasn't played yet
              players[req.tid] = -1;
              break;
            
            case 1: // already played
              first_temp = first_play_queue;
              last_temp = last_play_queue;
              while (first_temp != last_temp) {
                if (play_queue[first_temp].tid == req.tid) {
                  while (first_temp != last_temp) {
                    play_queue[i].tid = play_queue[i+1].tid;
                    play_queue[i].move = play_queue[i+1].move;
                    first_temp = (first_temp + 1) % 50;
                  }
                  break;
                }
                first_temp = (first_temp + 1) % 50;
              }
              last_play_queue = (last_play_queue - 1) % 50;
              players[req.tid] = -1;
              break;
            
            default:
              players[req.tid] = -1;
              break;
          }
        default:
          break;
      }

      Reply(sender_tid, &result, sizeof(struct ns_request));
    }
}

