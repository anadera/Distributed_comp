#include "lab4.h"

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

typedef struct llist_t {
Message* value;
struct llist_t* next;
struct llist_t* prev;
} llist_t;

llist_t* list_create ( Message* msg ) {
	llist_t* t = malloc (sizeof(llist_t) );
	t-> value = msg;
	t-> next = NULL;
	t-> previous = NULL;
	return t;
}

llist_t* list_add (Message* msg, llist_t* t) {
	llist_t* temp = list_create(msg);
	t-> next = temp;
	temp->prev = t;
	return temp;
}

llist_t* list_get (llist_t* t, Message* msg) {
	llist_t* temp = (llist_t*)t;
	llist_t* new;
	if (temp->next == NULL && temp->prev == NULL)
		return NULL;
	while (temp != NULL) {
		if (temp->next == NULL && temp->prev != NULL) {
			if (temp->value == msg){
				new = temp->prev;
				new->next = NULL;
				temp = new;
			}
			else
				temp=temp->prev;
		}
		else if (temp->next != NULL && temp->prev == NULL) {
			if (temp->value == msg){
				new = temp->next;
				new->prev = NULL;
				temp = new;
			}
			else 
				temp=temp->next;
		}
		else if (temp->next != NULL && temp->prev != NULL) {
			if (temp->value == msg){
				new = temp->prev;
				new->next=temp->next;
				new = temp->next;
				new->prev = temp->prev;
				temp = new;
			}
			else
				temp = new;
		}
	}
	return temp;
}

int list_is_min_time (llist_t* t, timestamp_t time){
	llist_t* temp = (llist_t*)t;
	while (temp!=NULL){
		if (time >= temp->value->s_header->s_local_time)
			return 0;
		temp=temp->prev;
	}
		return 1;
}

llist_t* list_node_at (size_t value, const llist_t* const t) { 
	size_t i=0;
	llist_t* temp = (llist_t*)t;
	while (temp != NULL ) {
		if (i == value ) return temp;
		temp = temp->next; i++;
		
	}
	return NULL;
	
}

int request_cs(const void * self){
	Message* msg;
	PROCESS* p = (PROCESS*)self;
	msg.s_header = (MessageHeader) {
                .s_magic = MESSAGE_MAGIC,
                .s_payload_len = 0,
                .s_type = CS_REQUEST,
                .s_local_time = get_lamport_time()
        };
	list_add_back(msg);
	for (int i=1; i<=p->x; i++) {
		if (i==p->id)
			continue;
		send(p,i,msg);
	}
	

}

int enter_cs(const void * self);
	

int release_cs(const void * self);

