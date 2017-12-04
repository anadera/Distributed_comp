#pragma once

#include <unistd.h> //pipe fork close getpid getppid
#include <sys/types.h> //wait getpid getppid
#include <sys/wait.h> //wait
#include <stdlib.h> //malloc exit atoi free
#include <stdio.h> //perror
#include <time.h> //time
#include <errno.h>
#include "io.h"
#include "common.h"
#include "pa2345.h"
#include "ipc.h"

//#include <string.h> //strcmp
#define SUCCESS  0
#define FAILURE -1

static const char * const p_fd_fmt =
    "%d: Process %1d (pid %5d, parent %5d) has READ fd %5d and WRITE fd %5d \n";
static const char * const wrong_argument =
  	"%d: process %1d received wrong argument for parent_step\n";

typedef struct p{
  FILE * events;
  FILE * pipes;
  int id;
  int x;
  llist_t* queue;
  int fd[][2];
} PROCESS;

void create_pipe(int size, int array[][2]);
void set_fd(int array[][2], PROCESS * p);

timestamp_t get_lamport_time();
void set_time(timestamp_t msg_time);
void update_time();
int parent_step(PROCESS* p, int type);
int child_step(PROCESS* p, int type);
int create_child(int fds[][2], pid_t* pids, PROCESS* p);
