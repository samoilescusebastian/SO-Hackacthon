#include <stdlib.h>
#include "node.h"
#ifndef LIST
#define LIST

typedef node_t *list_iterator_t;

typedef struct list {
	node_t *head;
	size_t size;
	/*
	 * functii generice pentru printare, eliberarea si compararea unui nod
	 */
	void (*free_func)(void *data);
	void (*print_func)(void *data);
	char (*compare_func)(void *data_1, void *data_2);
} list_t;

char push_back(list_t *list, void *data);
char push_front(list_t *list, void *data);
char insert_multiple(list_t *list, list_t *new_elements, int pos);
char pop_back(list_t *list);
char pop_front(list_t *list);
void print_list(list_t *list);
void free_list(list_t *list);
char delete_element(list_t *list, void *data);
list_iterator_t get_list_it(list_t *list);
char has_next_list_it(list_iterator_t it);
void *get_next_list_it(list_iterator_t *it);
list_t *create_string_list(void);

#endif
