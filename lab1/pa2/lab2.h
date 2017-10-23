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
#include "banking.h"

//#include <string.h> //strcmp
#define SUCCESS  0
#define FAILURE -1

static const char * const p_fd_fmt =
    "%d: Process %1d (pid %5d, parent %5d) has READ fd %5d and WRITE fd %5d \n";
static const char * const wrong_argument =
  	"%d: process %1d received wrong argument for parent_step\n";

typedef struct p{
	int id;
	int x;
	int fd[][2];
} PROCESS;

typedef struct f{
  FILE * events;
  FILE * pipes;
} FILENAME;

void create_pipe(int size, int array[][2]);
void set_fd(int array[][2], PROCESS * p);

void set_start_balance(local_id self, BalanceHistory* h, int * array);
void set_balance(BalanceHistory* history, balance_t amount);
//int handle_transfer(PROCESS* p, Message * msgIN, BalanceHistory* h, FILENAME* f);

int parent_step(PROCESS* p, FILENAME* f, int type);
int parent_work(PROCESS* p);
int parent_after_done(PROCESS* p);
void child_step(PROCESS* p, FILENAME* f, BalanceHistory* h, int* array);
int child_work(PROCESS* p, FILENAME* f, BalanceHistory* h);
int create_child(int fds[][2], pid_t* pids, PROCESS* p, FILENAME* f, int * array);
