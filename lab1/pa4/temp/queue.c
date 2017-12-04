#include "queue.h"

node_t* create_node ( timestamp_t t, local_id i){
	node_t* node = malloc (sizeof(node_t));
	node->id = i;
	node->time = t;
	return node;
}

llist_t* list_create ( node_t* node ) {
        llist_t* t = malloc (sizeof(llist_t) );
        t-> node = node;
        t-> next = NULL;
        t-> prev = NULL;
        return t;
}

llist_t* list_add (node_t* node, llist_t* t) {
        llist_t* temp = list_create(node);
        t-> next = temp;
        temp->prev = t;
        return temp;
}

llist_t* list_get (node_t* node, llist_t* t) {
        llist_t* temp = (llist_t*)t;
        llist_t* new;
        if (temp->next == NULL && temp->prev == NULL)
                return NULL;
        while (temp != NULL) {
                if (temp->next == NULL && temp->prev != NULL) {
                        if (temp->node->id == node->id && temp->node->time == node->time){
                                new = temp->prev;
                                new->next = NULL;
                                temp = new;
                        }
                        else
                                temp=temp->prev;
                }
                else if (temp->next != NULL && temp->prev == NULL) {
                        if (temp->node->id == node->id && temp->node->time == node->time){
                                new = temp->next;
                                new->prev = NULL;
                                temp = new;
                        }
                        else
                                temp=temp->next;
                }
                else if (temp->next != NULL && temp->prev != NULL) {
                        if (temp->node->id == node->id && temp->node->time == node->time){
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

/* return 1 if node is MIN or 0 if node is NOT min */
int list_is_min_time (node_t* node, llist_t* t){
        llist_t* temp = (llist_t*)t;
        while (temp!=NULL){
                if (node->time > temp->node->time)
                        return 0;
		else if (node->time == temp->node->time && node->id > temp->node->id)
			return 0;
                temp=temp->prev;
        }
	return 1;
}

void list_print(llist_t* t) {
	llist_t* temp = (llist_t*)t;
	while (temp->node!=NULL){
		printf ("Node id=%d time=%d\n", temp->node->id, temp->node->time);
		temp=temp->prev;
	}
}

int main(int argc, char** argv){
	int m1, m2;
	llist_t* l = list_create( create_node (1,1) );
	list_add ( create_node (2,1), l );
	list_add ( create_node (2,2), l );
	list_add ( create_node (3,3), l );
	list_print (l);
	printf ("---------\n");
	list_get ( create_node (3,3), l );
	list_print (l);
	m1 = list_is_min_time ( create_node (2,2), l);
	m2 = list_is_min_time ( create_node (2,1), l);
	printf ("m1=%d m2=%d\n", m1, m2);
	return 0;
}	
