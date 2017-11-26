#include "lab2.h"

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

void set_balance(BalanceHistory* history, balance_t amount, time_t msg_time){
	timestamp_t time = get_physical_time();
	balance_t past_balance = history->s_history_len == 0 ? 0 :  history->s_history[history->s_history_len-1].s_balance;
  	// printf("past_balance = %d\n", past_balance);
	timestamp_t from = history->s_history_len;
	for (timestamp_t t = from; t<=time; t++){
		history->s_history[t] = (BalanceState) {
			.s_time = t,
			.s_balance = past_balance,
			.s_balance_pending_in = 0
		};
		//printf("t=%d history->s.history[t].s_time=%d history->s.history[t].s_balance=%d\n", t, history->s_history[t].s_time, history->s_history[t].s_balance);
	}
	for (timestamp_t t = msg_time; t<=time; t++){
    		history->s_history[t].s_balance += amount;
  	}
	history->s_history_len = time+1; 
	// printf("time=%d history->s.history[time].s_time=%d history->s.history[time].s_balance=%d\n", time, history->s_history[time].s_time, history->s_history[time].s_balance);
 	// printf("history->s_history_len = %d\n", history->s_history_len);
 }
int wait_for_ack(void * parent_data, local_id dst){
        PROCESS* p = (PROCESS*)parent_data;
        Message msg;
        while (receive(p,dst,&msg)) {
                if (msg.s_header.s_type == ACK) {
                       return 0; 
                }
        }
        return 1;
}

void transfer(void * parent_data, local_id src, local_id dst, balance_t amount){
	PROCESS* p = (PROCESS*)parent_data;
	Message msg;
  	msg.s_header = (MessageHeader) {
  		.s_magic = MESSAGE_MAGIC,
  		.s_payload_len = sizeof(TransferOrder),
  		.s_type = TRANSFER,
  		.s_local_time = get_physical_time()
  	};
	TransferOrder order = (TransferOrder){
		.s_src = src,
		.s_dst = dst,
		.s_amount = amount
	};
  	memcpy(msg.s_payload, &order, sizeof(TransferOrder));
	send(p,src,&msg);
	while (wait_for_ack(p, dst) != 0); 
}

