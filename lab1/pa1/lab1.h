#ifndef __LAB1_H__
#define __LAB1_H__

#include <unistd.h> //pipe fork close getpid getppid
#include <sys/types.h> //wait getpid getppid
#include <sys/wait.h> //wait
#include <stdlib.h> //malloc exit atoi free
#include <stdio.h> //perror
#include <time.h> //time
//#include <string.h> //strcmp
#define SUCCESS  0
#define FAILURE -1

static const char * const p_fd_fmt =
    "Process %1d (pid %5d, parent %5d) has READ fd %5d and WRITE fd %5d \n";

static const FILE* const des_events_log;
static const FILE* const des_pipes_log;

typedef struct p{
	int id;
	int x;
	int fd[][2];
} PROCESS;

void create_pipe(int size, int array[][2]);
void set_fd(int array[][2], PROCESS * p);
int wait_child(int* array, PROCESS* p);
void parent_step1(PROCESS* p);
void parent_step3(PROCESS* p);
void child_step1(PROCESS* p);
void child_step3(PROCESS* p);
int create_child(int array[][2], pid_t* pids, PROCESS* p);

#endif
