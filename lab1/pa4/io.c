#include "io.h"

/*
parse command line arguments
return array from "./lab2 -p X array[0] array[1] ..."
*/

/*
int parse_x(int argc, char** argv, int * array){
	int d;
    if(!strcmp(argv[1], "-p")){
		if (!
		  ((d = atoi(argv[2]))>1 &&
		  d <=10 && d == (argc-3))
		   )
		{
			perror("parse_x:wrong X");
			exit(EXIT_FAILURE);
		}
	}
	else {
		printf("parse_x:wrong key");
		exit(-1);
    }
	for (int i=0; i<d; i++){
	    if (!
	    ((array[i] = atoi(argv[i+3])) > 0) &&
	    ((array[i] = atoi(argv[i+3])) <=99)
	    )
	    {
	        perror("parse_x:wrong sum");
			exit(EXIT_FAILURE);
	    }
	}
    return d;
}
*/

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
fill declared Message struct
*/
/*
void create_msg(Message msg, MessageType type, char * body, int id, balance_t balance, timestamp_t time){
	char tmp[MAX_PAYLOAD_LEN] = "";
	size_t buf = 0;
	switch (type){
		case STARTED:
			buf = sprintf(tmp, body, time, id, getpid(), getppid(), balance);
			strncpy(msg.s_payload, tmp, buf);
			break;
		case DONE:
			buf = sprintf(tmp, body, time, id, balance);
			strncpy(msg.s_payload, tmp, buf);
			break;
		case TRANSFER:
			buf = sizeof(TransferOrder);
			memcpy(msg.s_payload, body, buf);
			break;
		case BALANCE_HISTORY:
			buf = sizeof(BalanceHistory);
			memcpy(msg.s_payload, body, buf);
			break;
		case ACK:
			buf = sprintf(tmp, body, time, id, getpid(), getppid(), balance);
			strncpy(msg.s_payload, tmp, buf);
			break;
		default:
			strncpy(msg.s_payload, body, buf);
			break;
	}
	msg.s_header = (MessageHeader) {
		.s_magic = MESSAGE_MAGIC,
		.s_payload_len = buf,
		.s_type = type,
		.s_local_time = time
	};
}
*/
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
