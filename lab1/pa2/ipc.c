#include "ipc.h"
#include "lab2.h"

/** Send a message to the process specified by id.
 *
 * @param self    Any data structure implemented by students to perform I/O
 * @param dst     ID of recepient
 * @param msg     Message to send
 *
 * @return 0 on success, any non-zero value on error
 */

 const int block = 8192; //64 Kbit

int send(void * self, local_id dst, const Message * msg){
	PROCESS *p = (PROCESS*)self;
	//int count = strlen(msg);
  if (p->id == dst){
    perror("send:src=dst\n");
    return FAILURE;
  }
	int fd = p->fd[dst][1];
  printf("process id=%d send msg type=%d to dst=%d\n", p->id, msg->s_header.s_type, fd);
  size_t size = sizeof(msg->s_header) + msg->s_header.s_payload_len;
	int status = write(fd, msg, size);
  return status > 0 ? SUCCESS : FAILURE;
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
	//int count = strlen(msg);
	int size = p->x;
	for (i=0; i<=size; i++){
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

int receive(void * self, local_id from, Message * msg){
	PROCESS *p = (PROCESS*)self;
  if (p->id == from)
    return FAILURE;
	int fd = p->fd[from][0];
	int read_bytes = read(fd, msg, sizeof(Message));
	if (read_bytes < 0 && errno != EAGAIN) {
        perror("receive:read");
        return FAILURE;
  }
  return read_bytes > 0 ? SUCCESS : FAILURE;
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
  char buff[MAX_MESSAGE_LEN];
	int size = p->x;
  int des = 0;
  int read_bytes = 0;
  int i;
  while(1){
    for (i=0; i<=size; i++){
      printf("receive_any: i=%d\n", i);
      des=p->fd[i][0];
      if (des == 0 || des == NULL)
        continue;
      printf("receive_any: des=%d\n", des);
      read_bytes = read(des,buff, MAX_MESSAGE_LEN);
      if (read_bytes>0){
        memcpy(msg,buff,read_bytes);
        return SUCCESS;
      }
      else
        continue;
    }
    usleep(10000);
  }
  return FAILURE;
}
