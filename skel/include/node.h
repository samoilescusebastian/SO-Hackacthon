#ifndef NODE
#define NODE

typedef struct node {
	char *data;
	struct node *next;
} node_t;

node_t *push_node_back(node_t *head, void *data);
node_t *push_node_front(node_t *head, void *data);
void free_node(node_t *node, void (*free_func)(void *));
void free_nodes(node_t *head, void (*free_func)(void *));
void print_nodes(node_t *head, void(*print_data)(void *data));
node_t *delete_node(node_t *head, void *value, void (*free_func)(void *),
		    char (*comp_func)(void*, void*));
node_t *remove_last(node_t *head, void(*free_func)(void *));
node_t *remove_first(node_t *head, void(*free_func)(void *));
#endif
