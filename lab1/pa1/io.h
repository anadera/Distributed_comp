#ifndef __IO_H__
#define __IO_H__

#include "ipc.h"
#include <stdlib.h> //atoi exit
#include <string.h> //strcpy strcmp strlen
#include <stdio.h> //perror printf fprintf
#include <time.h> //time
#include <unistd.h> //getpid getppid
#include <sys/types.h> //getpid getppid

int parse_x(char** argv);
void create_msg(Message msg, MessageType type, const char * const body);
void log_events(const char * const fmt, int self, FILE* des);

#endif
