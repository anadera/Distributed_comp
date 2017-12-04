#define _GNU_SOURCE
#include <fcntl.h>
#include "lab4.h"

void create_pipe(int size, int array[][2]){
	for (int i=0; i<size; i++){
		if (pipe2(array[i], O_NONBLOCK ) == -1){
			perror("create_pipe is failed");
			exit (EXIT_FAILURE);
		}
	}
}

void set_fd(int array[][2], PROCESS * p){
	int i;
	int skip = 0;
	int id = p->id;
	int x = p->x;
	int size = x*(x+1);

	for(i=0; i<size; i++){
		if (i/x == id) {		//READ fds
			if(i%x == id)
				skip = 1;
			p->fd[i%x+skip][0] = array[i][0];
			close(array[i][1]);
		}
		else if (i/x != id){	//WRITE fds
			if(i/x < id && i%x+1 == id){
				p->fd[i/x][1] = array[i][1];
				close(array[i][0]);
			}
			else if (i/x > id && i%x == id){
				p->fd[i/x][1] = array[i][1];
				close(array[i][0]);
			}
			else {
				close(array[i][0]);
				close(array[i][1]);
			}
		}
	}
}

int parent_step(PROCESS* p, int type){
	static const char * fmt;
	int status = 0;
	Message msg = { {0} };
	int num = p->x;
	int self = p->id;
	FILE * des = p->events;
	while(1) {
		while((receive_any((void*)p,&msg) == 0) && msg.s_header.s_type == type) {
			set_time(msg.s_header.s_local_time);
			update_time();
			status++;
			break;
		}
		if (status == num)
			break;
	}
	switch(type){
		case STARTED:
			fmt = log_received_all_started_fmt;
			break;
		case DONE:
			fmt = log_received_all_done_fmt;
			break;
		default:
			fmt = wrong_argument;
			break;
	}
	printf(fmt, get_lamport_time(), self);
	fprintf(des, fmt, get_lamport_time(), self);
	return SUCCESS;
}

int child_step(PROCESS* p, int type){
	Message msg = { {0} };
	Message msgIN = { {0} };
	int self = p->id;
	int num = p->x;
	FILE* des = p->events;
	char tmp[MAX_PAYLOAD_LEN] = "";
	size_t buf;
	update_time();
	switch (type) {
		case(STARTED):
			buf = sprintf(tmp,log_started_fmt,get_lamport_time(), self, getpid(), getppid(),0);
			break;
		case(DONE):
			buf = sprintf(tmp,log_done_fmt,get_lamport_time(), self, 0);
			break;
		default:
			buf = 0;
			break;
	}
	strncpy(msg.s_payload, tmp, buf);
	msg.s_header = (MessageHeader) {
                .s_magic = MESSAGE_MAGIC,
                .s_payload_len = buf,
                .s_type = type,
                .s_local_time = get_lamport_time()
        };
	send_multicast((void*)p, (const Message *)&msg);
	printf(tmp);
	fprintf(des,tmp);
	for (int i=1; i<=num; i++){
		if (i != self && i !=0 ) {
			while((receive((void*)p,i,&msgIN) == 0) && msgIN.s_header.s_type == type) {
				set_time(msgIN.s_header.s_local_time);
				update_time();
				printf("%d: msg type=%d is received, time is updated\n", get_lamport_time(),type);
			}
		}
	}
	switch (type) {
	case (STARTED):
		printf(log_received_all_started_fmt,get_lamport_time(),self);
		fprintf(des, log_received_all_started_fmt,get_lamport_time(),self);
		break;
	case (DONE):
		printf(log_received_all_done_fmt,get_lamport_time(),self);
		fprintf(des, log_received_all_done_fmt,get_lamport_time(),self);
		break;
	default:
		break;
	}
	return SUCCESS;
}

void child_work(PROCESS* p) {
	int N = p->id * 5 ;
	for (int i = 1; i <= N; i++){
		//request_cs();
		print(log_loop_operation_fmt);
		//release_cs();
	}
}

int create_child(int fds[][2], pid_t* pids, PROCESS* p){
	int size = p->x;
	printf ("size = %d\n",size);
	int id = 0;
	for (pid_t i=0; i<size; i++){
		if ((pids[i] = fork() ) == 0) {
			/* Child process */
			p->id = i+1;
			id = i+1;
			set_fd(fds,p); //p.fd содержит полезную инф для чилдов
			for (pid_t j=0;j<=size*(size+1);j++){
				if (j==id) continue;
				log_pipes(p_fd_fmt,p->id,p->fd[j][0],p->fd[j][1], p->pipes);
			}
			if ( child_step(p,STARTED) != 0){
				perror("child step STARTED\n");
				return FAILURE;
			}
			child_work(p);
			if ( child_step(p,DONE) != 0) {
				perror("child step DONE\n");
				return FAILURE;
			}
			exit(EXIT_SUCCESS);
		}
		else if (pids[i] < 0) {
			/* Fail process */
			perror("create_process:child");
			exit(EXIT_FAILURE);
		}
	}
	/* Parent process */
	set_fd(fds,p); //p.fd содержит полезную инф для парента и чилдов
	for(pid_t i=0;i<=size;i++){
		if (i==id) continue;
		log_pipes(p_fd_fmt,p->id,p->fd[i][0],p->fd[i][1], p->pipes);
	}
	//step 1
	if (parent_step(p, STARTED) != 0)
		return FAILURE;
	//step 2
	//step 3
	if (parent_step(p, DONE) != 0)
		return FAILURE;
	printf("BEFORE WAIT PID\n");
	for (int j=0; j<size; j++){
		waitpid(pids[j], NULL,0);
		printf("WAIT PID\n");
	}
	return SUCCESS;
}

int main(int argc, char* argv[]){
	PROCESS * p;
	int pipes_num; //number of pipes
	int size_x;
	CM_ARGUMENTS* cmArgs;
	cmArgs = malloc(sizeof(cmArgs));
	size_x = parse_cm(argc,argv,cmArgs);	
	pid_t* pid; //array of children' pids
	pid = (pid_t *)malloc(sizeof(pid_t)*size_x);
	pipes_num = size_x*(size_x+1);
	int fds[pipes_num][2]; //array of pipes' fds
	p = (PROCESS *)malloc(sizeof(PROCESS)*pipes_num);
	p->x = size_x;
	p->id = 0;
	p->events = fopen(events_log, "w+");
	p->pipes = fopen(pipes_log, "w+");
	create_pipe(pipes_num,fds); //fds != p.fd  fds передаем set_fd   //works fine
	int r = create_child(fds,pid,p);
	fclose((FILE *)p->events);
	fclose((FILE*)p->pipes);
	free((void*)p);
	free((void*)pid);
	return r;
}
