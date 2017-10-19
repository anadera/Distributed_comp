#include "lab1.h"
#include "io.h"
#include "common.h"
#include "pa1.h"
#include "ipc.h"

/*
size - number of pipes,
array - point on array of file descriptors
*/
void create_pipe(int size, int array[][2]){
	int i;
    for (i=0; i<size; i++){
		if (pipe(array[i]) == -1){
			perror("create_pipe is failed");
			exit (EXIT_FAILURE);
		}
	}
}

/*
x - number of children,
size - number of pipes,
array - array of fds,
p points on inf about read and write fds for every child
*/
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

/*
size - number of children
array - points on array of id of determinated children
status - points on array of children statuses
*/
int wait_child(int* array, PROCESS* p){
	int size = p->x;
	while (size > 0){
		while ((array[size] = wait(NULL)) > 0);
		size--;
	}
	return SUCCESS;
}

void parent_step1(PROCESS* p){
	Message msg = { {0} };;
	int self = p->id;
	int num = p->x;
	while(num > 0){
		if (num != self)
			while(receive((void*)p,num,&msg) != 0);
		num--;
	}
	log_events(log_received_all_started_fmt,self, des_events_log);
}

//void parent_step2(){}

void parent_step3(PROCESS* p){
	Message msg = { {0} };;
	int self = p->id;
	int num = p->x;
	while(num > 0){
		if (num != self){
			Message msg;
			while(receive((void*)p,num,&msg) != 0);
		}
		num--;
	}
	log_events(log_received_all_done_fmt,self, des_events_log);
}

void child_step1(PROCESS* p){
	Message msg = { {0} };;
	Message msgIN = { {0} };;
	int self = p->id;
	int num = p->x;
	log_events(log_started_fmt,self, des_events_log);
	create_msg(msg,STARTED,log_started_fmt);
	send_multicast((void*)p, &msg);

	while(num > 0){
		if (num != self)
			while(receive((void*)p,num,&msgIN) != 0);
		num--;
	}
	log_events(log_received_all_started_fmt,self, des_events_log);
}

/* void child_step2(){} */

void child_step3(PROCESS* p){
	Message msg = { {0} };;
	Message msgIN = { {0} };;
	int self = p->id;
	int num = p->x;
	log_events(log_done_fmt,self, des_events_log);
	create_msg(msg,DONE,log_done_fmt);
	send_multicast((void*)p,&msg);

	while(num > 0){
		if (num != self)
			while(receive((void*)p,num,&msgIN) != 0);
		num--;
	}
	log_events(log_received_all_done_fmt,self, des_events_log);
}

/*
size - number of children
array - point on array with children pids
*/
int create_child(int array[][2], pid_t* pids, PROCESS* p){
	pid_t i, j;
	int size = p->x;
	int array_dc[size] = NULL; //array of id of determinated children
	int id = p->id;
	for (i=1; i<=size; ++i){
		if ((pids[i] = fork()) == 0) {
			/* Child process */

			set_fd(array,p); //p.fd содержит полезную инф для чилдов
			log_pipes(p_fd_fmt,i,p->fd[i][0],p->fd[i][1], des_pipes_log);

			child_step1(p);
			//child_step2();
			child_step3(p);
			exit(EXIT_SUCCESS);
		}
		else if ((pids[i] = fork()) == -1){
			/* Fail process */
			perror("create_process:child");
			return FAILURE;
		}
	}

	/* Parent process */

	set_fd(array,p); //p.fd содержит полезную инф для парента и чилдов
	for(j=0;j<=p->x;j++){
		if (j==id) continue;
		log_pipes(p_fd_fmt,j,p->fd[j][0],p->fd[j][1], des_pipes_log);
	}

	parent_step1(p);
	//parent_step2();
	parent_step3(p);
	return (wait_child(array_dc,p));

}

int main(int argc, char* argv[]){
	PROCESS * p;
	int x; //number of child processes
	int N; // children+parent
	int pipes_num; //number of pipes

	pid_t pid[x]; //array of children' pids
	x = parse_x(argv); //works fine
	N = x + 1;
	pipes_num = N*(N-1);
	int fds[pipes_num][2]; //array of pipes' fds
	p = (PROCESS *)malloc(sizeof(PROCESS)*pipes_num);
	p->x=x;
	p->id = PARENT_ID;

	static const FILE* const des_events_log = fopen(events_log, "w+");
	static const FILE* const des_pipes_log = fopen(pipes_log, "w+");

	create_pipe(pipes_num,fds); //fds != p.fd  fds передаем set_fd   //works fine
	if (create_child(fds,pid,p) == SUCCESS){
		fclose(des_events_log);
		fclose(des_pipes_log);
		free((void*)p);
		exit(EXIT_SUCCESS);
	}
	else
		exit(EXIT_FAILURE);
}
