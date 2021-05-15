#include <string.h>
#include <stdio.h>

#include "include/hashmap.h"
#include "include/list.h"
#include "include/struct_utils.h"

static list_t **alloc_buckets(size_t size)
{
	size_t i;
	list_t **buckets = (list_t **)malloc(sizeof(list_t *) * size);

	if (buckets == NULL)
		return NULL;

	for (i = 0; i < size; i++) {
		buckets[i] = (list_t *)malloc(sizeof(list_t));
		if (buckets[i] == NULL)
			return NULL;
		init_hashlist(buckets[i]);
	}

	return buckets;
}

static char insert_into_bucket(hashmap_t *hashmap, hashmap_node_t *node)
{
	size_t hash = str_hash(node->key, hashmap->capacity);

	list_t *bucket = hashmap->buckets[hash];
	char result_code;

	if (bucket == NULL)
		return ERROR;

	result_code = push_back(bucket, node);
	if (result_code == ERROR)
		return ERROR;

	hashmap->size++;
	return SUCCESS;
}

static void free_buckets(list_t **buckets, size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
		free_list(buckets[i]);

	free(buckets);
}

size_t str_hash(const char *string, size_t capacity)
{
	unsigned int hash = FIRSTH;
	size_t i = 0;

	while (string[i] != '\0') {
		hash = (hash * A) ^ (string[i] * B);
		i++;
	}

	return hash % capacity;
}

hashmap_node_t *create_hashmap_node(char *key, void *value, size_t size_value)
{
	hashmap_node_t *map_node =
		(hashmap_node_t *)malloc(sizeof(hashmap_node_t));

	if (map_node == NULL)
		return NULL;

	map_node->key = (char *)malloc(strlen(key) + 1);
	map_node->value = (char *)malloc(size_value);
	if (map_node->key == NULL || map_node->value == NULL)
		return NULL;

	strcpy(map_node->key, key);
	memcpy(map_node->value, value, size_value);
	map_node->is_active = 1;

	return map_node;
}

char comp_hashmap_node(void *data_1, void *data_2)
{
	hashmap_node_t *node_1 = (hashmap_node_t *)data_1;
	hashmap_node_t *node_2 = (hashmap_node_t *)data_2;

	return strcmp(node_1->key, node_2->key) == 0 &&
		node_1->compare_value(node_1->value, node_2->value) &&
		node_1->is_active == node_2->is_active;
}

void print_hashmap_node(void *data)
{
	hashmap_node_t *node = (hashmap_node_t *)data;
	char *value_to_string = node->print_value(node->value);
	printf("key = %s, value = %s, is_active = %s\n", node->key,
	value_to_string, node->is_active == 1 ? TRUE : FALSE);

	free(value_to_string);
}

void free_hashmap_node(void *data)
{
	hashmap_node_t *node = (hashmap_node_t *)data;

	free(node->key);
	node->free_value(node->value);
	free(node);
}

/*
 *initializeaza list asociata fiecarei bucket cu functii de print,
 *free si comparare corespunzatoare nodurilor din hash
 */
void init_hashlist(list_t *list)
{
	list->head = NULL;
	list->size = 0;
	list->compare_func = comp_hashmap_node;
	list->print_func = print_hashmap_node;
	list->free_func = free_hashmap_node;
}

hashmap_t *create_hashmap(size_t inital_capacity, char (*compare_func)(void *data_1, void *data_2),
						void (*free_func)(void *data), char *(*print_func)(void *data), size_t size_value)
{
	hashmap_t *hashmap = (hashmap_t *)malloc(sizeof(hashmap_t));
	hashmap->compare_value = compare_func;
	hashmap->free_value = free_func;
	hashmap->print_value = print_func;
	hashmap->size_value = size_value;
	
	if (hashmap == NULL)
		return NULL;

	hashmap->capacity = inital_capacity;
	hashmap->size = 0;
	hashmap->buckets = alloc_buckets(inital_capacity);
	if (hashmap->buckets == NULL)
		return NULL;


	return hashmap;
}

hashmap_node_t *get_node(hashmap_t *hashmap, char *key)
{
	size_t hash = str_hash(key, hashmap->capacity);
	list_t *bucket = hashmap->buckets[hash];

	list_iterator_t it = get_list_it(bucket);
	hashmap_node_t *node = NULL;

	while (has_next_list_it(it)) {
		node = get_next_list_it(&it);
		if (strcmp(node->key, key) == 0)
			return node;

	}

	return NULL;
}

char push(hashmap_t *hashmap, char *key, void *value)
{
	hashmap_node_t *node;
	hashmap_node_t *target_node;
	list_t **old_buckets;
	char *handler;
	char result_code;
	size_t old_capacity;
	list_iterator_t it;
	size_t i;
	/*
	 * daca cheia este prezenta in hash, se face update la valoare curenta
	 */
	target_node = get_node(hashmap, key);
	if (target_node != NULL) {
		target_node->is_active = 1;
		if (handler == NULL)
			return ERROR;
		target_node->value = handler;
		memcpy(target_node->value, value, hashmap->size_value);
	} else {
		/*
		 * se aloca un nod ce va fi adaugat in hashmap
		 */
		node = create_hashmap_node(key, value, hashmap->size_value);
		node->free_value = hashmap->free_value;
		node->print_value = hashmap->print_value;
		node->compare_value = hashmap->compare_value;
		if (node == NULL)
			return ERROR;
		/*
		 * daca hash-ul a ajuns la capacitatea sa maxima
		 * se incearca o realocare a bucket-urilor
		 */
		if (hashmap->size == hashmap->capacity) {
			old_capacity = hashmap->capacity;
			old_buckets = hashmap->buckets;
			hashmap->capacity = (old_capacity << 1);
			hashmap->buckets = alloc_buckets(hashmap->capacity);
			hashmap->size = 0;
			if (hashmap->buckets == NULL) {
				free_buckets(old_buckets, old_capacity);
				return ERROR;
			}

			for (i = 0; i < old_capacity; i++) {
				it = get_list_it(old_buckets[i]);
				while (has_next_list_it(it)) {
					target_node = get_next_list_it(&it);
					if (target_node == NULL)
						break;
					result_code = push(hashmap,
						      target_node->key,
						      target_node->value);

					if (result_code == ERROR) {
						free_buckets(old_buckets,
							    old_capacity);
						return ERROR;
					}
				}
			}

			free_buckets(old_buckets, old_capacity);
		}
		/*
		 * se introduce in hash nodul nou creat
		 */
		result_code = insert_into_bucket(hashmap, node);
		if (result_code == ERROR)
			return ERROR;
	}

	return SUCCESS;
}

char remove_node(hashmap_t *hashmap, hashmap_node_t *node)
{
	size_t hash = str_hash(node->key, hashmap->capacity);
	list_t *bucket = hashmap->buckets[hash];
	size_t result_code = delete_element(bucket, node);

	if (result_code == ERROR)
		return ERROR;

	bucket->size--;
	hashmap->size--;
	return SUCCESS;
}

char remove_key(hashmap_t *hashmap, char *key)
{
	hashmap_node_t *node = get_node(hashmap, key);
	size_t result_code;

	if (node == NULL)
		return SUCCESS;

	result_code = remove_node(hashmap, node);
	return result_code == ERROR;
}

void free_hashmap(hashmap_t *hashmap)
{
	free_buckets(hashmap->buckets, hashmap->capacity);
	free(hashmap);
}

hashmap_iterator_t get_hashmap_it(hashmap_t *hashmap)
{
	hashmap_iterator_t hm_it =
	(hashmap_iterator_f *)malloc(sizeof(hashmap_iterator_f));

	hm_it->index = 0;
	hm_it->capacity = hashmap->capacity;
	hm_it->it = get_list_it(hashmap->buckets[0]);
	hm_it->buckets = hashmap->buckets;

	return hm_it;
}

char has_next_hashmap_it(hashmap_iterator_t it)
{
	return it->index < it->capacity;
}

hashmap_node_t *get_next_hashmap_it(hashmap_iterator_t *it)
{
	hashmap_node_t *node;
	/*
	 * se incearca returnarea elementului urmator din bucket-ul urmator
	 * daca bucket-ul curent a ajuns la sfarsit, se obtine urmatorul
	 */
	if (has_next_list_it((*it)->it))  {
		node = (hashmap_node_t *)get_next_list_it(&((*it)->it));
	} else {
		(*it)->index = (*it)->index + 1;
		if ((*it)->index  >= (*it)->capacity)
			return NULL;
		(*it)->it = get_list_it((*it)->buckets[(*it)->index]);
		node = get_next_hashmap_it(it);
	}

	return node;
}



