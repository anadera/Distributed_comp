#include "io.h"
#include "ipc.h"

/*
parse command line arguments
return X from "./lab2 -p X"
*/
int parse_x(char** argv){
	int d;
	if(!strcmp(argv[1], "-p")){
		if ((d = atoi(argv[2]))>0 && (d = atoi(argv[2])) <=10)
			return d;
		else {
			perror("parse_x:wrong X");
			exit(EXIT_FAILURE);
		}
			
	}	
	else {
		perror("parse_x:wrong key");
		exit(EXIT_FAILURE);
  }
}

/*
fill declared Message struct
*/
void create_msg(Message msg, MessageType type, const char * const body){	
	msg.s_header = (MessageHeader) {
		.s_magic = MESSAGE_MAGIC,
		.s_payload_len = strlen(body),
		.s_type = type,
		.s_local_time = time(0)
	};
	strcpy(msg.s_payload, body);
}

/*
log event to stdout and in file
fmt - event_message
self - local process id
des - descriptor of opened file
*/
void log_events(const char * const fmt, int self, int des){
	if (printf(fmt,self,getpid(),getppid()) < 0)
		perror("log_events:printf");	
	if (fprintf((FILE*)des,fmt,self,getpid(),getppid()) < 0)
		perror("log_events:fprintf");	
}
