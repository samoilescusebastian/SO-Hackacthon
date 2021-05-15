#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "include/node.h"

node_t *push_node_back(node_t *head, void *data, node_t** tail)
{
	node_t *it;

	if (head == NULL) {
		node_t *node = (node_t *)(malloc(sizeof(node_t)));
		*tail = node;

		if (node == NULL)
			return NULL;

		node->data = data;
		node->next = NULL;
		return node;
	}

	it = head;
	while (it->next != NULL)
		it = it->next;

	it->next = (node_t *)(malloc(sizeof(node_t)));
	if (it->next == NULL)
		return NULL;

	it->next->data = data;
	it->next->next = NULL;
	*tail = it->next;

	return head;
}

node_t *push_node_front(node_t *head, void *data)
{
	node_t *new_node = (node_t *)malloc(sizeof(node_t));

	if (new_node == NULL)
		return NULL;

	new_node->data = data;
	new_node->next = head;
	head = new_node;

	return head;
}

void free_node(node_t *node, void (*free_func)(void *))
{
	free_func(node->data);
	free(node);
}

void free_nodes(node_t *head, void (*free_func)(void *))
{
	node_t *aux;

	while (head != NULL) {
		aux = head;
		head = head->next;
		free_node(aux, free_func);
	}
}

void print_nodes(node_t *head, void(*print_data)(void *data))
{
	node_t *iter = head;

	while (iter != NULL) {
		print_data(iter->data);
		iter = iter->next;
	}
}

node_t *delete_node(node_t *head, node_t** tail, void *value, void (*free_func)(void *),
		    char (*comp_func)(void*, void*))
{
	node_t *prev_iter;
	node_t *curr_iter = head;

	if (curr_iter == NULL)
		return NULL;

	if (comp_func(curr_iter->data, value)) {
		curr_iter = head->next;
		if (curr_iter == *tail) {
			*tail = NULL;
		}
		free_node(head, free_func);
		return curr_iter;
	}

	prev_iter = curr_iter;
	curr_iter = curr_iter->next;
	while (curr_iter != NULL) {
		if (comp_func(curr_iter->data, value)) {
			if (curr_iter == *tail) {
				*tail = prev_iter;
			}
			prev_iter->next = curr_iter->next;
			free_node(curr_iter, free_func);
			break;
		}
		prev_iter = curr_iter;
		curr_iter = curr_iter->next;
	}

	return head;
}

node_t *remove_last(node_t *head, node_t** tail, void(*free_func)(void *))
{
	node_t *iter = head;

	if (head == NULL)
		return NULL;

	if (head->next == NULL) {
		free_node(head, free_func);
		tail = NULL;
		return NULL;
	}

	while (iter->next->next != NULL)
		iter = iter->next;
	*tail = iter;
	free_node(iter->next, free_func);
	iter->next = NULL;

	return head;
}

node_t *remove_first(node_t *head, void(*free_func)(void *))
{
	node_t *aux;

	if (head == NULL)
		return NULL;

	aux = head->next;
	free_node(head, free_func);
	return aux;

}
