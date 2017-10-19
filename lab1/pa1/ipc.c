#include "ipc.h"
/** Send a message to the process specified by id.
 *
 * @param self    Any data structure implemented by students to perform I/O
 * @param dst     ID of recepient
 * @param msg     Message to send
 *
 * @return 0 on success, any non-zero value on error
 */  
int send(void * self, local_id dst, const Message * msg){
	PROCESS *p = (PROCESS*)self;
	size_t count = strlen(msg);
	int fd = p.fds[dst][1];
	if (write(fd, msg, count) <0){
		perror("send");
		return FAILURE;
	}
	else
		return SUCCESS;
}

/** Send multicast message.
 *
 * Send msg to all other processes including parrent.
 * Should stop on the first error.
 * 
 * @param self    Any data structure implemented by students to perform I/O
 * @param msg     Message to multicast.
 *
 * @return 0 on success, any non-zero value on error
 */
int send_multicast(void * self, const Message * msg){
	int i;
	PROCESS *p = (PROCESS*)self;
	size_t count = strlen(msg);	
	size_t size = p.x;
	for (i=0; i<=size;i++){		
		send((void*)p,i,msg);	
	}
	return SUCCESS;
}

/** Receive a message from the process specified by id.
 *
 * Might block depending on IPC settings.
 *
 * @param self    Any data structure implemented by students to perform I/O
 * @param from    ID of the process to receive message from
 * @param msg     Message structure allocated by the caller
 *
 * @return 0 on success, any non-zero value on error
 */
 
const size_t block = 8192; //64 Kbit
 
int receive(void * self, local_id from, Message * msg){
	PROCESS *p = (PROCESS*)self;
	int file_size;
	int fd = p.fds[from][0];
	while((read_bytes = read(p.fd, msg, block)) > 0) {
        file_size+=read_bytes;                
    }
    if (file_size>0)
		return SUCCESS;
	else
		return FAILURE;
}

/** Receive a message from any process.
 *
 * Receive a message from any process, in case of blocking I/O should be used
 * with extra care to avoid deadlocks.
 *
 * @param self    Any data structure implemented by students to perform I/O
 * @param msg     Message structure allocated by the caller
 *
 * @return 0 on success, any non-zero value on error
 */
int receive_any(void * self, Message * msg){
	PROCESS *p = (PROCESS*)self;
	int i;	
	size_t size = p.x;
	for (i=0; i<size;i++){
		receive((void*)p,i, msg);
	}
	return SUCCESS;
}
