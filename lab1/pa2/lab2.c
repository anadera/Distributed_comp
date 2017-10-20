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
	for (int i=0; i<size; i++){
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

int parent_step(PROCESS* p, FILENAME* f, const char * const fmt){
	Message msg = { {0} };
	int self = p->id;
	int num = p->x;
	FILE * des = f->events;
	for (int i=0; i<=num; i++){
		if (i != self)
			while(receive((void*)p,i,&msg) != 0);
	}
	log_events(fmt,self, des);
	return SUCCESS;
}

void child_step(PROCESS* p, FILENAME* f, const char * const fmt_OUT, const char * const fmt_IN){
	Message msg = { {0} };
	Message msgIN = { {0} };
	int self = p->id;
	int num = p->x;
	FILE * des = f->events;
	log_events(fmt_OUT,self, des);
	create_msg(msg,STARTED,fmt_OUT, self);
	send_multicast((void*)p, (const Message *)&msg);
	for (int i=1; i<=num; i++){
		if (i != self && i !=0 )
			while(receive((void*)p,i,&msgIN) != 0);
	}
	log_events(fmt_IN,self, des);
}

/*
size - number of children
array - point on array with children pids
*/
int create_child(int array[][2], pid_t* pids, PROCESS* p, FILENAME * f){
	int size = p->x;
	int id = 0;
	for (pid_t i=0; i<size; i++){
		if ((pids[i] = fork() ) == 0) {
			/* Child process */
			p->id = i+1;
			id = i+1;
			set_fd(array,p); //p.fd содержит полезную инф для чилдов
			for (pid_t j=0;j<=size;j++){
				if (j==id) continue;
				log_pipes(p_fd_fmt,id,p->fd[j][0],p->fd[j][1], f->pipes);
			}
			child_step(p, f, log_started_fmt, log_received_all_started_fmt);
			child_step(p, f, log_done_fmt, log_received_all_done_fmt);
			exit(EXIT_SUCCESS);
		}

		else if (pids[i] < 0) {
			/* Fail process */
			perror("create_process:child");
			exit(EXIT_FAILURE);
		}
	}

	/* Parent process */
	set_fd(array,p); //p.fd содержит полезную инф для парента и чилдов
	for(pid_t i=0;i<=size;i++){
		if (i==id) continue;
		log_pipes(p_fd_fmt,id,p->fd[i][0],p->fd[i][1], f->pipes);
	}

	if (parent_step(p, f, log_received_all_started_fmt))
		//bank_robbery();
	parent_step(p, f, log_received_all_done_fmt);
	for (int j=0; j<size; j++){
		waitpid(pids[j], NULL,0);
	}
	return SUCCESS;
}

int main(int argc, char* argv[]){
	PROCESS * p;
	FILENAME * f;
	int x; //number of child processes
	int pipes_num; //number of pipes
	x = parse_x(argv); //works fine
	pid_t* pid; //array of children' pids
	pid = (pid_t *)malloc(sizeof(pid_t)*x);
	pipes_num = x*(x+1);
	int fds[pipes_num][2]; //array of pipes' fds
	p = (PROCESS *)malloc(sizeof(PROCESS)*pipes_num);
	f = (FILENAME *)malloc(sizeof(FILENAME));
	p->x=x;
	p->id = 0;

	f->events = fopen(events_log, "w+");
	f->pipes = fopen(pipes_log, "w+");

	create_pipe(pipes_num,fds); //fds != p.fd  fds передаем set_fd   //works fine
	int r = create_child(fds,pid,p,f);
	fclose((FILE *)f->events);
	fclose((FILE*)f->pipes);
	free((void*)p);
	free((void*)f);
	free((void*)pid);
	return r;
}
