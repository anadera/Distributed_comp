#include "io.h"

int parse_cm(int argc, char** argv, CM_ARGUMENTS* cmArgs){
        int option_index = 0;
        int opt = 0;
        static struct option longOpt[] = {
                {"proc", required_argument, 0, 'p'},
                {"mutexl", no_argument, 0, 'm'},
                {NULL, 0, 0, 0}
        };
        while (( opt = getopt_long(argc, argv, "p:m", longOpt, &option_index)) > 0) {
                switch (opt) {
                case 'm':
                        cmArgs->mutexIsUsed = 1;
                        break;
                case 'p':
                        cmArgs->x = atoi(optarg);
                        if ( cmArgs->x > MAX_PROCESS_ID || cmArgs->x <= 0)
                                return -1;
                        break;

                default:
                        break;
                }
        }
        return cmArgs->x;
}
/*
log event to stdout and in file
fmt - event_message
self - local process id
des - descriptor of opened file
*/

void log_pipes(const char * const fmt, int self, int fd_R, int fd_W, FILE* des){
	if (printf(fmt, get_lamport_time(),self,getpid(),getppid(), fd_R,fd_W) < 0)
		perror("log_events:printf");
	if (fprintf((FILE*)des,fmt,get_lamport_time(),self,getpid(),getppid(), fd_R, fd_W) < 0)
		perror("log_events:fprintf");
}
