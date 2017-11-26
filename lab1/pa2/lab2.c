#define _GNU_SOURCE
#include <fcntl.h>
#include "lab2.h"
/*
size - number of pipes,
array - point on array of file descriptors
*/
void create_pipe(int size, int array[][2]){
	for (int i=0; i<size; i++){
		if (pipe2(array[i], O_NONBLOCK | O_DIRECT) == -1){
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

int parent_step(PROCESS* p, int type){
	static const char * fmt;
	int status = 0;
	Message msg = { {0} };
	int num = p->x;
	FILE * des = p->events;
	while(1) {
		while((receive_any((void*)p,&msg) == 0) && msg.s_header.s_type == type) {
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

//	if (status == num) {
		printf(fmt, get_physical_time(), p->id);
		fprintf(des, fmt, get_physical_time(), p->id);
		return SUCCESS;
//	}
//	else
//		return FAILURE;
}

int parent_work(PROCESS* p){
	Message msg = { {0} };
	//int self = p->id;
	int num = p->x;
	//printf("%d: call bank_robbery, self=%d, num=%d\n", get_physical_time(), self, num);
	bank_robbery((void *)p, num);
	//create_msg(msg, STOP, NULL, self,0);
	msg.s_header = (MessageHeader) {
		.s_magic = MESSAGE_MAGIC,
		.s_payload_len = 0,
		.s_type = STOP,
		.s_local_time = get_physical_time()
	};
	send_multicast((void*)p, (const Message *)&msg); //send STOP to all childs
	//printf("%d: process %d send STOP\n", get_physical_time(), self);
	return SUCCESS;
}

void preprocess_all_his(AllHistory * ah){
  int max_time=0;
  for(uint8_t i=0; i < ah->s_history_len; ++i)
    max_time = max_time >= ah->s_history[i].s_history_len ? max_time : ah->s_history[i].s_history_len;
  for(uint8_t i=0; i < ah->s_history_len; ++i)
  {    
    for (uint8_t j = 1; j < max_time; ++j)
    {
      if ( ah->s_history[i].s_history[j].s_time == 0 ){
        ah->s_history[i].s_history[j].s_time = j;
        ah->s_history[i].s_history[j].s_balance += ah->s_history[i].s_history[j-1].s_balance;
      }
    }
    ah->s_history[i].s_history_len=max_time;
  }
}


int parent_after_done(PROCESS* p){
	Message msgIN = { {0} };
	int num = p->x;
	int status = 0;
	AllHistory ah;
	memset(&ah, 0, sizeof(AllHistory));
	ah.s_history_len = num;
	//preprocess_all_his(&ah);
	while(1) {
		while((receive_any((void*)p,&msgIN) == 0) && msgIN.s_header.s_type == BALANCE_HISTORY) {
			memcpy((void*)&ah.s_history[status],&msgIN.s_payload,msgIN.s_header.s_payload_len);
			for (int j=0; j<ah.s_history[status].s_history_len; j++) 
				//printf ("BalanceHistory[%d].s_history[%d] s_balance = %d s_time = %d\n", status, j, ah.s_history[status].s_history[j].s_balance, ah.s_history[status].s_history[j].s_time);
			status++;	
			break;
		}
		if (status == num)
			break;
	}	
	//if (status == num) {
		//printf("%d: parent receive all BH\n", get_physical_time());
	preprocess_all_his(&ah);
	print_history(&ah);
	return SUCCESS;
	//}
	//else
	//	return FAILURE;
}

void child_step(PROCESS* p, BalanceHistory* h, int * array){
	Message msg = { {0} };
	Message msgIN = { {0} };
	balance_t start_balance = 0;
	int self = p->id;
	int num = p->x;
	FILE* des = p->events;
	set_start_balance(self, h, array);
	start_balance = h->s_history[0].s_balance;
	//printf("%d: process %d has start_balance %d\n", get_physical_time(), self, start_balance);
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

int child_work(PROCESS* p, BalanceHistory* h){
	local_id self = p->id;
	int num = p->x;
	int done_counter = 0;
	Message msg;
	Message msgBH;
	size_t buf;
	TransferOrder order;
	memset(&msg, 0, sizeof(msg));
	memset(&msgBH, 0, sizeof(msgBH));
	balance_t fin_balance;
	int status;
	char tmp[MAX_PAYLOAD_LEN] = "";
	//printf("start child_work\n");
	while (1){
		//printf("WHILE ITERATION\n");
		status = receive_any((void *)p, &msg);
		if (status != 0) {
			printf ("child %d does not receive any msg\n", self);
			return FAILURE;
		}
		//printf("%d: process %d receive MSG type=%d\n", get_physical_time(),self,msg.s_header.s_type);
		switch (msg.s_header.s_type){
			case (TRANSFER):
				memcpy(&order, msg.s_payload, msg.s_header.s_payload_len);
				//printf("%d: child id=%d receive TRANSFER src=%d dst=%d\n", get_physical_time(),self,order.s_src,order.s_dst);
				if (order.s_src == self){
					//fprintf(p->events,log_transfer_in_fmt,get_physical_time(),p->id,order.s_amount,0);
					//printf(log_transfer_in_fmt,get_physical_time(),p->id,order.s_amount,0);
					set_balance(h, -(order.s_amount));
					if (send(p,order.s_dst,(const Message *)&msg) != 0){
						perror("send TRANSFER is failed");
						exit(EXIT_FAILURE);
					}
					fprintf(p->events,log_transfer_out_fmt,get_physical_time(),p->id,order.s_amount,order.s_dst);
					printf(log_transfer_out_fmt,get_physical_time(),p->id,order.s_amount,order.s_dst);
					//printf("%d: process %d send TRANSFER=%d to %d\n", get_physical_time(),self,msg.s_header.s_type, order.s_dst);
				}
				else  if (order.s_dst == self) {
					fprintf(p->events,log_transfer_in_fmt,get_physical_time(),p->id,order.s_amount,order.s_src);
					printf(log_transfer_in_fmt,get_physical_time(),p->id,order.s_amount,order.s_src);
					msg.s_header = (MessageHeader) {
						.s_magic = MESSAGE_MAGIC,
						.s_payload_len = 0,
						.s_type = ACK,
						.s_local_time = get_physical_time()
					};
					set_balance(h, order.s_amount);
					if (send(p, PARENT_ID, (const Message *)&msg) != 0){
						perror("send ACK is failed");
						exit(EXIT_FAILURE);
					}
					//fprintf(p->events,log_transfer_out_fmt,get_physical_time(),p->id,order.s_amount,order.s_dst);
					//printf(log_transfer_out_fmt,get_physical_time(),p->id,order.s_amount,order.s_dst);
					//printf("%d: process %d send ACK=%d to 0\n", get_physical_time(),self,msg.s_header.s_type);
				}
				else {
					printf("error is occured in switch-transfer\n");
				}
				break;
			case (STOP):
				//printf("%d: child id=%d receive STOP=%d\n", get_physical_time(),self,msg.s_header.s_type);
				done_counter++;
				fin_balance = h->s_history[h->s_history_len-1].s_balance;
				buf = sprintf(tmp, log_done_fmt, get_physical_time(), self, fin_balance);
				msg.s_header = (MessageHeader) {
					.s_magic = MESSAGE_MAGIC,
					.s_payload_len = buf,
					.s_type = DONE,
					.s_local_time = get_physical_time()
				};
				strncpy(msg.s_payload, tmp, buf);
				//create_msg(msg, DONE,(char *)log_done_fmt, self, fin_balance);
				send_multicast((void*)p, (const Message *)&msg);
				//printf("%d: child id=%d send DONE=%d\n", get_physical_time(),self,msg.s_header.s_type);
				//printf(log_done_fmt, get_physical_time(),self,fin_balance);
				break;
			case (DONE):
				done_counter++;
				if (done_counter == num){
					msgBH.s_header = (MessageHeader) {
						.s_magic = MESSAGE_MAGIC,
						.s_payload_len = sizeof *h - (MAX_T + 1 - h->s_history_len) * sizeof *h->s_history,
						.s_type = BALANCE_HISTORY,
						.s_local_time = get_physical_time()
					};
					memcpy(msgBH.s_payload, h, msgBH.s_header.s_payload_len);
					//create_msg(msgBH,BALANCE_HISTORY,(char *)&h,self,0);
					send(p, PARENT_ID,(const Message *)&msgBH);
					//printf("%d: child id=%d send BALANCE_HISTORY=%d\n", get_physical_time(),self,msgBH.s_header.s_type);
					exit(EXIT_SUCCESS);
				}
				break;
			default:
				//printf("%d: child id=%d receive msg type=%d\n", get_physical_time(),self, msg.s_header.s_type);
				break;
		}
		//printf("switch is finished\n");
	}
	return SUCCESS;
}

/*
size - number of children
fds - point on array with children pids
array - point on array of start_balance
*/
int create_child(int fds[][2], pid_t* pids, PROCESS* p, int* array){
	int size = p->x;
	//printf("create_Child_size: %d\n", size);
	int id = 0;
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
			for (pid_t j=0;j<=size*(size+1);j++){
				if (j==id) continue;
				log_pipes(p_fd_fmt,p->id,p->fd[j][0],p->fd[j][1], p->pipes);
				//printf("array[%d][0]=%d array[%d][1]=%d\n", j, p->fd[j][0], j, p->fd[j][1]);
			}

			child_step(p, &bh, array);
			int status = child_work(p,&bh);
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
		log_pipes(p_fd_fmt,p->id,p->fd[i][0],p->fd[i][1], p->pipes);
		//printf("array[%d][0]=%d array[%d][1]=%d\n", i, p->fd[i][0], i, p->fd[i][1]);
	}
	//step 1
	if (parent_step(p, STARTED) != 0)
		return FAILURE;
	//step 2
	parent_work(p);
	//step 3
	if (parent_step(p, DONE) != 0)
		return FAILURE;
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
	int pipes_num; //number of pipes

	int size_x = atoi(argv[2]);
	int array_x[size_x];
	if (argc <4){
		perror("wrong args number");
		exit(EXIT_FAILURE);
	}
	else if (atoi(argv[2]) <2 || atoi(argv[2]) >10){
		perror("wrong X, X: [2;10]");
		exit(EXIT_FAILURE);
	}
	else {
		for (int i=3; i<argc; i++){
			if (atoi(argv[i]) <1 || atoi(argv[i])> 99)			{
				perror("wrong S, S: [1;99]");
				exit(EXIT_FAILURE);
			}
			else {
				array_x[i-3]=atoi(argv[i]);
			}
		}
	}
	//int *array = &array_x;
	printf("main:size_x = %d\n", size_x);
	/* for (int lol=0; lol<size_x; lol++){
		printf("x[%d] = %d\n", lol,x[lol]);
	} */
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
	int r = create_child(fds,pid,p,array_x);
	fclose((FILE *)p->events);
	fclose((FILE*)p->pipes);
	free((void*)p);
	free((void*)pid);
	return r;
}
