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
		default:
			fmt = wrong_argument;
			break;
	}
	printf(fmt,get_physical_time(),self);
	fprintf(des,fmt,get_physical_time(),self);
	return SUCCESS;
}

int parent_work(PROCESS* p){
	Message msg = { {0} };
	int self = p->id;
	int num = p->x;
	printf("%d: call bank_robbery, self=%d, num=%d\n", get_physical_time(), self, num);
	bank_robbery((void *)p, num);
	create_msg(msg, STOP, NULL, self,0);
	send_multicast((void*)p, (const Message *)&msg); //send STOP to all childs
	printf("%d: process %d send STOP\n", get_physical_time(), self);
	return SUCCESS;
}

int parent_after_done(PROCESS* p){
	Message msgIN = { {0} };
	int self = p->id;
	int num = p->x;
	AllHistory all = { 0 };
	for (int i=0; i<num; i++){
		if (i != self) {
			while( (receive((void*)p,i,&msgIN) != 0) &&
					(msgIN.s_header.s_type == BALANCE_HISTORY) ){
				memcpy(&all.s_history[i], &msgIN.s_payload, msgIN.s_header.s_payload_len);
				all.s_history_len = all.s_history_len + 1;
			}
		}
	}
	print_history(&all);
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
	start_balance = h->s_history[0].s_balance;
	printf("%d: process %d has start_balance %d\n", get_physical_time(), self, start_balance);
	create_msg(msg,STARTED,(char *)log_started_fmt, self,0);
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

int child_work(PROCESS* p, FILENAME* f, BalanceHistory* h){
	local_id self = p->id;
	int num = p->x;
	int done_counter = 0;
	Message msg;
	memset(&msg, 0, sizeof msg);
	balance_t fin_balance;
	printf("start child_work\n");
	while (1){
		int mail = receive_any((void *)p, &msg);
		if(mail != 0)
			return FAILURE;
		printf("%d: process is %d child_work.receive_any: %d\n", get_physical_time(), self, msg.s_header.s_type);
		switch (msg.s_header.s_type){
			case (TRANSFER):
				printf("%d: process id=%d receive TRANSFER\n", get_physical_time(),h->s_id);
				memcpy(&order, msg.s_payload, msg.s_header.s_payload_len);
				if (order->s_src == self){
					set_balance(h, order->s_amount);
					status = send((void*)p,order.s_dst,&msg);
					if (status != 0)
						return FAILURE;
				}
				else {
					msg.s_header = (MessageHeader) {
						.s_magic = MESSAGE_MAGIC,
						.s_payload_len = 0,
						.s_type = ACK,
						.s_local_time = get_physical_time()
					};
					set_balance(h, -(order->s_amount));
					status = send(p,0, &msg);
				}
				break;
			case (STOP):
				fin_balance = h->s_history[h->s_history_len].s_balance;
				create_msg(msg, DONE,(char *)log_done_fmt, self, fin_balance);
				send_multicast((void*)p, (const Message *)&msg);
				printf(log_done_fmt, get_physical_time(),self,fin_balance);
				break;
			case (DONE):
				done_counter++;
				if (done_counter == num-1){
					Message msgBH = { {0} };
					create_msg(msgBH,BALANCE_HISTORY,(char *)&h,self,0);
					send(p, PARENT_ID,(const Message *)&msgBH);
					exit(EXIT_SUCCESS);
				}
				break;
		}
	}
	return SUCCESS;
}

/*
size - number of children
fds - point on array with children pids
array - point on array of start_balance
*/
int create_child(int fds[][2], pid_t* pids, PROCESS* p, FILENAME * f, int* array){
	int size = p->x;
	int id = 0;
	/*
	int size_x = sizeof(&array)/sizeof(array[0]);
	for (int lol=0; lol<size_x; lol++){
		printf("x[%d] = %d\n", lol,array[lol]);
	} */
	for (pid_t i=0; i<size; i++){
		if ((pids[i] = fork() ) == 0) {
			/* Child process */
			p->id = i+1;
			id = i+1;
			BalanceHistory bh = {
				.s_id = p->id,
				.s_history_len = 0,
				.s_history = { {0} }
			};
			set_fd(fds,p); //p.fd содержит полезную инф для чилдов
			for (pid_t j=0;j<=size;j++){
				if (j==id) continue;
				log_pipes(p_fd_fmt,p->id,p->fd[j][0],p->fd[j][1], f->pipes);
			}
			child_step(p, f, &bh, array);
			int status = child_work(p, f, &bh);
			if (status != 0)
				return FAILURE;
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
		log_pipes(p_fd_fmt,p->id,p->fd[i][0],p->fd[i][1], f->pipes);
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
	int pipes_num; //number of pipes
	int* x = parse_x(argc, argv); //number of child processes
	int size_x = sizeof(&x)/sizeof(x[0]);
	/* for (int lol=0; lol<size_x; lol++){
		printf("x[%d] = %d\n", lol,x[lol]);
	} */
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
