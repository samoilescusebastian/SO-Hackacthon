#ifndef HASHMAP
#define HASHMAP

#include "list.h"

typedef struct hashmap_iterator {
	size_t index;
	size_t capacity;
	list_iterator_t it;
	list_t **buckets;
} hashmap_iterator_f;

typedef hashmap_iterator_f* hashmap_iterator_t;

typedef struct hashmap_node {
	char is_active;
	char *key;
	void *value;
	void (*free_value)(void *data);
	char* (*print_value)(void *data);
	char (*compare_value)(void *data_1, void *data_2);
} hashmap_node_t;

hashmap_node_t *create_hashmap_node(char *key, void *value, size_t size_value);

typedef struct hashmap {
	size_t size;
	size_t capacity;
	list_t **buckets;
	size_t size_value;
	void (*free_value)(void *data);
	char *(*print_value)(void *data);
	char (*compare_value)(void *data_1, void *data_2);

} hashmap_t;

size_t str_hash(const char *string, size_t capacity);
char comp_hashmap_node(void *data_1, void *data_2);
void print_hashmap_node(void *data);
void free_hashmap_node(void *data);
void init_hashlist(list_t *list);
hashmap_t *create_hashmap(size_t inital_capacity, char (*compare_func)(void *data_1, void *data_2),
						void (*free_func)(void *data), char *(*print_func)(void *data), size_t size_value);
hashmap_node_t *get_node(hashmap_t *hashmap, char *key);
char push(hashmap_t *hashmap, char *key, void *value);
char remove_node(hashmap_t *hashmap, hashmap_node_t *node);
char remove_key(hashmap_t *hashmap, char *key);
void free_hashmap(hashmap_t *hashmap);
hashmap_iterator_t get_hashmap_it(hashmap_t *hashmap);
char has_next_hashmap_it(hashmap_iterator_t it);
hashmap_node_t *get_next_hashmap_it(hashmap_iterator_t *it);

#endif
