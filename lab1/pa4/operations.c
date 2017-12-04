#include "lab4.h"
#include "queue.h"

static timestamp_t global_time = 0;

timestamp_t get_lamport_time() {
	return global_time;
}

void set_time(timestamp_t msg_time) {	
	global_time = ( global_time >  msg_time ? global_time : msg_time );	
}

void update_time() {
	global_time = global_time + 1;
}

int request_cs(const void * self){
	Message* msg;
	PROCESS* p = (PROCESS*)self;
	msg.s_header = (MessageHeader) {
                .s_magic = MESSAGE_MAGIC,
                .s_payload_len = 0,
                .s_type = CS_REQUEST,
                .s_local_time = get_lamport_time()
        };
	if (p->queue == NULL)
		list_create(msg);
	else
		list_add(msg, p->queue);
	for (int i=1; i<=p->x; i++) {
		if (i==p->id)
			continue;
		send(p,i,msg);
	}
	return SUCCESS;	
}

int enter_cs(const void * self);
	

int release_cs(const void * self);

