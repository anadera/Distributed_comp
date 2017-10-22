#pragma once

#include "ipc.h"
#include "pa2345.h"
#include "banking.h"
#include <stdlib.h> //atoi exit
#include <string.h> //strcpy strcmp strlen
#include <stdio.h> //perror printf fprintf
#include <time.h> //time
#include <unistd.h> //getpid getppid
#include <sys/types.h> //getpid getppid

int * parse_x(int argc, char** argv);
void create_msg(Message msg, MessageType type, char * body, int id, balance_t balance);
void log_pipes(const char * const fmt, int self, int fd_R, int fd_W, FILE* des);
