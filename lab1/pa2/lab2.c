#include "lab2.h"

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

int parent_step(PROCESS* p, FILENAME* f, int type){
	static const char * fmt;
	Message msg = { {0} };
	int self = p->id;
	int num = p->x;
	FILE * des = f->events;
	for (int i=0; i<num; i++){
		if (i != self)
			while((receive((void*)p,i,&msg) != 0) && msg.s_header.s_type == type);
	}
	switch(type){
		case STARTED:
			fmt = log_received_all_started_fmt;
			break;
		case DONE:
			fmt = log_received_all_done_fmt;
			break;
		case default:
			fmt = wrong_argument;
			break;
	}
	printf(fmt,get_physical_time(),self);
	fprintf(des,fmt,get_physical_time(),self);
	return SUCCESS;
}

int parent_work(PROCESS* p){
	Message msg = {{0}};
	int self = p->id;
	int num = p->x;
	bank_robbery((void *)p, num);
	create_msg(msg, STOP, NULL, self);
	send_multicast((void*)p, (const Message *)&msg); //send STOP to all childs
	return SUCCESS;
}

int parent_after_done(PROCESS* p){
	Message msgIN = { {0} };
	int self = p->id;
	int num = p->x;
	AllHistory all = {{{{{0}}}}};
	for (int i=0; i<num; i++){
		if (i != self) {
			while( (receive((void*)p,i,&msgIN) != 0) &&
					(msgIN.s_header.s_type == BALANCE_HISTORY) ){
				AllHistory all.s_history[i] = (BalanceHistory)msgIN.s_body;
				AllHistory all.s_history_len++;
			}
		}
	}
	print_history(all);
	return SUCCESS;
}

void child_step(PROCESS* p, FILENAME* f, BalanceHistory* h, int* array){
	Message msg = { {0} };
	Message msgIN = { {0} };
	balance_t start_balance = 0;
	int self = p->id;
	int num = p->x;
	FILE* des = f->events;
	set_start_balance(self, h, array);
	start_balance = h.s_history[self].s_balance;
	create_msg(msg,STARTED,log_started_fmt, self,0);
	send_multicast((void*)p, (const Message *)&msg);
	printf(log_started_fmt,get_physical_time(),self, getpid(), getppid(), start_balance);
	fprintf(des, log_started_fmt,get_physical_time(),self, getpid(), getppid(), start_balance);
	for (int i=1; i<=num; i++){
		if (i != self && i !=0 )
			while((receive((void*)p,i,&msgIN) &&
							msgIN.s_header.s_type == STARTED) != 0);
	}
	printf(log_received_all_started_fmt,get_physical_time(),self);
	fprintf(des, log_received_all_started_fmt,get_physical_time(),self);
}

void child_work(PROCESS* p, FILENAME* f, BalanceHistory* h){
	local_id self = p->id;
	int num = p->x;
	int done_counter = 0;
	while (1){
		Message msgIN = { {0} };
		if (receive_any(p, &msgIN) && msgIN.s_header.s_type == TRANSFER){
			handle_transfer(p,msgIN,h,f);
			continue;
		}
		else if (msgIN.s_header.s_type == STOP){
			Message msg = {{0}};
			balance_t fin_balance = h.s_history[h.s_history_len].s_balance;
			create_msg(msg, DONE,log_done_fmt, self, fin_balance);
			send_multicast(self, msg);
			printf(log_done_fmt, get_physical_time(),self,fin_balance);
		}
		else if (msgIN.s_header.s_type == DONE){
			done_counter++;
			if (done_counter == num-1){
				Message msgBH = {{0}};
				create_msg(msgBH,BALANCE_HISTORY,&h,self,0);
				send(self, PARENT_ID,(const Message *)&msgBH);
				exit(EXIT_SUCCESS);
			}
		}
		else
			continue;
	}
}

/*
size - number of children
fds - point on array with children pids
array - point on array of start_balance
*/
int create_child(int fds[][2], pid_t* pids, PROCESS* p, FILENAME * f, int* array){
	int size = p->x;
	int id = 0;
	for (pid_t i=0; i<size; i++){
		if ((pids[i] = fork() ) == 0) {
			/* Child process */
			BalanceHistory * bh = {{{0}}};
			p->id = i+1;
			id = i+1;
			set_fd(fds,p); //p.fd содержит полезную инф для чилдов
			for (pid_t j=0;j<=size;j++){
				if (j==id) continue;
				log_pipes(p_fd_fmt,id,p->fd[j][0],p->fd[j][1], f->pipes);
			}
			child_step(p, f, bh, fds, array);
			child_work(p, f, bh);
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
		log_pipes(p_fd_fmt,id,p->fd[i][0],p->fd[i][1], f->pipes);
	}
	//step 1
	parent_step(p, f, STARTED);
	//step 2
	parent_work(p);
	//step 3
	parent_step(p, f, DONE);
	//step 4
	parent_after_done(p);
	//step 5
	for (int j=0; j<size; j++){
		waitpid(pids[j], NULL,0);
	}
	return SUCCESS;
}

int main(int argc, char* argv[]){
	PROCESS * p;
	FILENAME * f;
	int* x; //number of child processes
	int pipes_num; //number of pipes
	x = parse_x(argv); //works fine
	int size_x = sizeof(&x)/sizeof(x[0]);
	pid_t* pid; //array of children' pids
	pid = (pid_t *)malloc(sizeof(pid_t)*size_x);
	pipes_num = size_x*(size_x+1);
	int fds[pipes_num][2]; //array of pipes' fds
	p = (PROCESS *)malloc(sizeof(PROCESS)*pipes_num);
	f = (FILENAME *)malloc(sizeof(FILENAME));
	p->x=size_x;
	p->id = 0;

	f->events = fopen(events_log, "w+");
	f->pipes = fopen(pipes_log, "w+");

	create_pipe(pipes_num,fds); //fds != p.fd  fds передаем set_fd   //works fine
	int r = create_child(fds,pid,p,f,x);
	fclose((FILE *)f->events);
	fclose((FILE*)f->pipes);
	free((void*)p);
	free((void*)f);
	free((void*)pid);
	return r;
}
