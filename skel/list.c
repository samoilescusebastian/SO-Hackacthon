#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "include/struct_utils.h"

char push_back(list_t *list, void *data)
{
	node_t *new_head;

	new_head = push_node_back(list->head, data);
	if (new_head == NULL)
		return ERROR;

	list->head = new_head;
	list->size++;
	return SUCCESS;
}

char push_front(list_t *list, void *data)
{
	list->head = push_node_front(list->head, data);
	if (list->head == NULL)
		return ERROR;

	list->size++;
	return SUCCESS;
}

/*
 * se face append intre list si new_elements, iar rezultatul se pune in list
 */
char insert_multiple(list_t *list, list_t *new_elements, int pos)
{
	node_t *it_1 = list->head;
	node_t *it_2 = new_elements->head;
	node_t *next;

	if (pos > list->size || new_elements == NULL) {
		if (new_elements != NULL)
			free_list(new_elements);
		return ERROR;
	}
	if (new_elements->size == 0) {
		free_list(new_elements);
		return SUCCESS;
	}

	if (pos == 0) {
		while (it_2->next != NULL)
			it_2 = it_2->next;

		it_2->next = list->head;
		list->head = new_elements->head;
	} else if (pos == list->size) {
		while (it_1->next != NULL)
			it_1 = it_1->next;

		it_1->next = new_elements->head;
	} else {
		while (pos > 1) {
			it_1 = it_1->next;
			pos--;
		}
		next = it_1->next;
		it_1->next = new_elements->head;
		while (it_2->next != NULL)
			it_2 = it_2->next;

		it_2->next = next;
	}

	list->size += new_elements->size;
	new_elements->head = NULL;
	free(new_elements);

	return SUCCESS;
}

char pop_back(list_t *list)
{
	list->head = remove_last(list->head, list->free_func);
	if (list->head == NULL)
		return ERROR;

	list->size--;
	return SUCCESS;
}

char pop_front(list_t *list)
{
	list->head = remove_first(list->head, list->free_func);
	if (list->head == NULL)
		return ERROR;

	list->size--;
	return SUCCESS;
}


void print_list(list_t *list)
{
	printf("\n");
	print_nodes(list->head, list->print_func);
	printf("\n");
}

void free_list(list_t *list)
{
	if (list == NULL)
		return;
	free_nodes(list->head, list->free_func);
	free(list);
}


char delete_element(list_t *list, void *data)
{

	list->head  = delete_node(list->head, data, list->free_func,
			      list->compare_func);
	list->size--;
	return SUCCESS;
}

list_iterator_t get_list_it(list_t *list)
{
	return list->head;
}

char has_next_list_it(list_iterator_t it)
{
	return it != NULL;
}

void *get_next_list_it(list_iterator_t *it)
{
	void *data = (*it)->data;
	(*it) = (*it)->next;

	return data;
}
