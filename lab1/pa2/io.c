#include "io.h"

/*
parse command line arguments
return array from "./lab2 -p X array[0] array[1] ..."
*/
int * parse_x(int argc, char** argv){
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
    int * array = (int*)malloc(sizeof(int)*d);
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
    return array;
}

/*
fill declared Message struct
*/
void create_msg(Message msg, MessageType type, char * body, int id, balance_t balance){
	char tmp[MAX_PAYLOAD_LEN] = "";
	size_t buf = 0;
	time_t time = get_physical_time();
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
		default:
			buf = 0;
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

/*
log event to stdout and in file
fmt - event_message
self - local process id
des - descriptor of opened file
*/

void log_pipes(const char * const fmt, timestamp_t time, int self, int fd_R, int fd_W, FILE* des){
	if (printf(fmt,self,getpid(),getppid(), fd_R,fd_W) < 0)
		perror("log_events:printf");
	if (fprintf((FILE*)des,fmt,self,getpid(),getppid(), fd_R, fd_W) < 0)
		perror("log_events:fprintf");
}
