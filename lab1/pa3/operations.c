#include "lab2.h"

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

void set_start_balance(local_id self, BalanceHistory* h, int* array){
	h->s_id = self;
	h->s_history_len = 1;
	h->s_history[0] = (BalanceState){
		.s_balance = array[self-1],
		.s_time = 0, //get_physical_time(),
		.s_balance_pending_in = 0
	};
	printf("t=0 history->s.history[0].s_time=%d history->s.history[0].s_balance=%d\n", h->s_history[0].s_time,
	h->s_history[0].s_balance);
}

void set_balance(BalanceHistory* history, balance_t amount, timestamp_t msg_time){
	//timestamp_t time = get_physical_time();
	timestamp_t time = get_lamport_time();
	printf("SET BALANCE: msg=%d curr=%d\n", msg_time, time);
	balance_t past_balance = history->s_history_len == 0 ? 0 :  history->s_history[history->s_history_len-1].s_balance;
	balance_t past_pending = history->s_history_len == 0 ? 0 :  history->s_history[history->s_history_len-1].s_balance_pending_in;
  	// printf("past_balance = %d\n", past_balance);
	timestamp_t from = history->s_history_len;
	for (timestamp_t t = from; t<=time; t++){
		history->s_history[t] = (BalanceState) {
			.s_time = t,
			.s_balance = past_balance,
			.s_balance_pending_in = past_pending 
		};
		//printf("t=%d history->s.history[t].s_time=%d history->s.history[t].s_balance=%d\n", t, history->s_history[t].s_time, history->s_history[t].s_balance);
	}
	for (timestamp_t t = msg_time; t<time; t++){
		history->s_history[t] = (BalanceState) {
			.s_time = t,
    			.s_balance = past_balance,
			.s_balance_pending_in = past_pending + ( amount > 0 ? amount : 0 )
		};
  	}
	history->s_history[time] = (BalanceState) {
		.s_time = time,
		.s_balance = past_balance+amount,
		.s_balance_pending_in = past_pending
	};
	
	//history->s_history[time].s_balance_pending_in = history->s_history[from-1].s_balance_pending_in;
	/*
	for (timestamp_t t = msg_time; t <= time; t++) {
		history->s_history[t].s_balance_pending_in = amount;
	}
	*/
	history->s_history_len = time+1; 
	// printf("time=%d history->s.history[time].s_time=%d history->s.history[time].s_balance=%d\n", time, history->s_history[time].s_time, history->s_history[time].s_balance);
 	// printf("history->s_history_len = %d\n", history->s_history_len);
 }
int wait_for_ack(void * parent_data, local_id dst){
        PROCESS* p = (PROCESS*)parent_data;
        Message msg;
        while (1) {
		int status = receive(p,dst,&msg);
                if ( status == 0 && msg.s_header.s_type == ACK) {
			set_time(msg.s_header.s_local_time);
			update_time();	
                	break; 
                }
		else
			continue;
        }
        return 0;
}

void transfer(void * parent_data, local_id src, local_id dst, balance_t amount){
	Message msg;
	//timestamp_t time;
	PROCESS* p = (PROCESS*)parent_data;
	update_time();
	//time = get_lamport_time();
  	msg.s_header = (MessageHeader) {
  		.s_magic = MESSAGE_MAGIC,
  		.s_payload_len = sizeof(TransferOrder),
  		.s_type = TRANSFER,
  		.s_local_time = get_lamport_time()
  	};
	TransferOrder order = (TransferOrder){
		.s_src = src,
		.s_dst = dst,
		.s_amount = amount
	};
  	memcpy(msg.s_payload, &order, sizeof(TransferOrder));
	send(p,src,&msg);
	if (wait_for_ack(p, dst) == 0) 
		printf("transfer: ack is received\n");
	else
		printf("transfer: ack is NOT received\n");	
}

