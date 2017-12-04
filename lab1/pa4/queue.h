#pragma once

#include "ipc.h"
#include <stdlib.h>

typedef struct node_t {
	timestamp_t time;
	local_id id;
} node_t;

typedef struct llist_t {
	node_t* node;
	struct llist_t* next;
	struct llist_t* prev;
} llist_t;

node_t* create_node (timestampt_t t, local_id i);
llist_t* list_create ( node_t* node );
llist_t* list_add ( node_t* node, llist_t* t );
llist_t* list_get ( node_t* node, llist_t* t );
int list_is_min_time ( node_t* node, llist_t* t );
void list_print(llist_t* t);

