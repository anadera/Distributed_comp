#include "lab2.h"

void set_start_balance(local_id self, BalanceHistory* h, int* array){
	h->s_id = self;
	h->s_history_len = 1;
	h->s_history[0] = (BalanceState){
		.s_balance = array[self-1],
		.s_time = get_physical_time(),
		.s_balance_pending_in = 0
	};
  printf("%d: set_start_balance: id = %d, balance = %d\n", get_physical_time(),h->s_id, h->s_history[0].s_balance);
}

void set_balance(BalanceHistory* history, balance_t amount){
	timestamp_t time = get_physical_time();
	balance_t past_balance = history->s_history[history->s_history_len-1].s_balance;
	timestamp_t gap_from = history->s_history_len;
	for (timestamp_t t = gap_from; t<time; t++){
		history->s_history[t] = (BalanceState) {
			.s_time = t,
			.s_balance = past_balance,
			.s_balance_pending_in = 0
		};
	}
	history->s_history[time] = (BalanceState) {
		.s_time = time,
		.s_balance = past_balance + amount,
		.s_balance_pending_in = 0
	};
	history->s_history_len = time+1;
}

void handle_transfer(PROCESS* p, Message * msgIN, BalanceHistory* h, FILENAME* f){
	Message msg = { {0} };
	TransferOrder order;
  memcpy(&order,&msgIN->s_payload, msgIN->s_header.s_payload_len);
	balance_t amount = order.s_amount;
  printf("%d: process id=%d handle TRANSFER with src=%d, dst=%d, amount=%d\n",get_physical_time(),p->id,order.s_src,order.s_dst,amount);
	FILE * des = f->events;
	int self = p->id;
	if (order.s_dst == self){
		set_balance(h, amount);
		create_msg(msg, ACK, NULL, self,0);
		send((void *)p, order.s_dst, (const Message *)&msg);
		printf(log_transfer_in_fmt, get_physical_time(), self, order.s_amount, order.s_src);
		fprintf(des,log_transfer_in_fmt, get_physical_time(), self, order.s_amount, order.s_src);
	}
	else {
		set_balance(h, -(amount));
		send((void*)p, order.s_dst, (const Message *)&msgIN);
		printf(log_transfer_out_fmt, get_physical_time(), self, order.s_amount, order.s_dst);
		fprintf(des, log_transfer_out_fmt, get_physical_time(), self, order.s_amount, order.s_dst);
	}
}

void transfer(void * parent_data, local_id src, local_id dst, balance_t amount){
  printf("transfer\n");
	PROCESS* p = (PROCESS*)parent_data;
	Message msg = {{0}};
	Message msgIN = {{0}};
	int self = p->id;
	TransferOrder order = (TransferOrder){
		.s_src = src,
		.s_dst = dst,
		.s_amount = amount
	};
	//create_msg(msg,TRANSFER,(char *)&order,self,amount);
  size_t buf = 0;
  time_t time = get_physical_time();
  buf = sizeof(TransferOrder);
  msg.s_header = (MessageHeader) {
  	.s_magic = MESSAGE_MAGIC,
  	.s_payload_len = buf,
  	.s_type = TRANSFER,
  	.s_local_time = time
  };
  memcpy(msg.s_payload, (void*)&order, buf);
  //
	send((void *)p,src,(const Message *)&msg);
  printf("%d: process id=%d send TRANSFER=%d to process=%d\n",get_physical_time(),self,msg.s_header.s_type,src);
	while (receive((void *)p,dst,&msgIN)){
		if (msgIN.s_header.s_type == ACK){
      printf("%d: process id=%d receive ACK from process=%d\n",get_physical_time(),self,dst);
			break;
    }
	}
}
