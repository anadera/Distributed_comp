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
/* int wait_child(int* array, PROCESS* p){
	int size = p->x;
	while (size > 0){
		while ((array[size] = wait(NULL)) > 0);
		size--;
	}

	return SUCCESS;
} */

void parent_step1(PROCESS* p, FILENAME* f){
	printf("parent step1\n");
	Message msg = { {0} };
	int self = p->id;
	int num = p->x;
	FILE * des = f->events;
	printf ("parent: self = %d, num = %d\n step1", self, num);
	while(num > 0){
		if (num != self)
			while(receive((void*)p,num,&msg) != 0);
		num--;
	}
	log_events(log_received_all_started_fmt,self, des);
}

//void parent_step2(){}

void parent_step3(PROCESS* p, FILENAME * f){
	printf("parent step3\n");
	int self = p->id;
	int num = p->x;
	FILE * des = f->events;
	Message msg = { {0} };
	printf ("parent: self = %d, num = %d\n step3", self, num);
	while(num > 0){
		if (num != self)
			while(receive((void*)p,num,&msg) != 0);
		num--;
	}
	log_events(log_received_all_done_fmt,self, des);
}

void child_step1(PROCESS* p, FILENAME* f){
	printf("child step1\n");
	Message msg = { {0} };
	Message msgIN = { {0} };
	FILE * des = f->events;
	int self = p->id;
	int num = p->x;
	printf ("child: self = %d, num = %d step1\n", self, num);
	log_events(log_started_fmt,self, des);
	create_msg(msg,STARTED,log_started_fmt, self);
	send_multicast((void*)p, (const Message *)&msg);
	printf ("child %d sent STARTED\n", getpid());

	while(num > 0){
		if (num != self)
			while(receive((void*)p,num,&msgIN) != 0);
		num--;
		printf("dec num as child %d received STARTED, num = %d\n", getpid(), num);
	}
	log_events(log_received_all_started_fmt,self, des);
}

/* void child_step2(){} */

void child_step3(PROCESS* p, FILENAME* f){
	printf("child step3\n");
	Message msg = { {0} };
	Message msgIN = { {0} };
	int self = p->id;
	int num = p->x;
	FILE * des = f->events;
	printf ("child: self = %d, num = %d step3\n", self, num);
	log_events(log_done_fmt,self, des);
	create_msg(msg,DONE,log_done_fmt, self);
	send_multicast((void*)p,(const Message *)&msg);
	printf ("child %d sent DONE\n", getpid());
	while(num > 0){
		if (num != self)
			while(receive((void*)p,num,&msgIN) != 0);
		num--;
		printf("dec num as child %d received DONE, num = %d\n", getpid(), num);
	}
	log_events(log_received_all_done_fmt,self, des);
}

/*
size - number of children
array - point on array with children pids
*/
int create_child(int array[][2], pid_t* pids, PROCESS* p, FILENAME * f){
	pid_t i, j;
	int size = p->x;
	//int array_dc[size]; //array of id of determinated children
	int id = 0;
	for (i=0; i<size; i++){
		pids[i] = fork();

		if (pids[i] > 0){
			/* Parent process */
			printf ("parent: p->id = %d, id = %d\n", p->id, id);
			set_fd(array,p); //p.fd содержит полезную инф для парента и чилдов
			printf ("parent set fd vipolnilos\n");
			for(j=0;j<=size;j++){
				if (j==id) continue;
				log_pipes(p_fd_fmt,id,p->fd[j][0],p->fd[j][1], f->pipes);
			}

			int k = p->x;
			while (k>0){
				printf("buit myaso\n");
				waitpid(pids[k], NULL,0);
				k--;
				printf("child %d zavershilsya\n", pids[k]);
			}

			printf ("parent zavershil cikl\n");

			printf ("parent pereshel k step1\n");
			parent_step1(p, f);
			printf ("parent %d pid %d zavershil step 1\n", id, getpid());
			//parent_step2();
			printf ("parent pereshel k step3\n");
			parent_step3(p, f);
			printf ("parent %d pid %d zavershil step 3\n", id, getpid());
		}

		else if (pids[i] == 0) {
			/* Child process */
			p->id = i+1;
			id = i+1;
			printf ("child: p->id = %d, id = %d\n", p->id, id);
			set_fd(array,p); //p.fd содержит полезную инф для чилдов
			printf ("child set fd vipolnilos\n");
			for (j=0;j<=size;j++){
				if (j==id) continue;
				log_pipes(p_fd_fmt,id,p->fd[j][0],p->fd[j][1], f->pipes);
			}
			printf ("child %d pid %d perehodit k step 1\n", id, getpid());
			child_step1(p, f);
			printf ("child %d pid %d zavershil step 1\n", id, getpid());
			//child_step2();
			child_step3(p, f);
			printf ("child %d pid %d zavershil step 3\n", id, getpid());
			exit(EXIT_SUCCESS);
		}

		else {
			/* Fail process */
			perror("create_process:child");
			return FAILURE;
		}

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
	//if (create_child(fds,pid,p,f) == SUCCESS){
		fclose(f->events);
		fclose(f->pipes);
		free((void*)p);
		free((void*)f);
		free((void*)pid);
		return r;
	//}
	//else
	//	exit(EXIT_FAILURE);
}
