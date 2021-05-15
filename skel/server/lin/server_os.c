/**
 * Hackathon SO: LogMemCacher
 * (c) 2020-2021, Operating Systems
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "../../include/list.h"

#include "../../include/server.h"

extern char *logfile_path;

static int client_function(SOCKET client_sock)
{
	int rc;
	struct logmemcache_client_st *client;

	client = logmemcache_create_client(client_sock);

	while (1) {
		rc = get_command(client);
		if (rc == -1)
			break;
	}

	close(client_sock);
	free(client);

	return 0;
}

char compare_thread(void *data_1, void *data_2) {
	pthread_t *thread_1 = (pthread_t*)data_1;
	pthread_t *thread_2 = (pthread_t*)data_2;

	return (*thread_1) == (*thread_2);
}

void print_thread(void *data) {
	pthread_t *thread = (pthread_t*)data;

	printf("%ld\n", *thread);
}

void free_thread(void *data) {
	pthread_t *thread = (pthread_t*)data;

	free(thread);
}

void *start_client_thread(void *data) {
	SOCKET socket = *(SOCKET*)data;

	client_function(socket);

	return NULL;
}

list_t* create_thread_list() {
	list_t *list = (list_t *)malloc(sizeof(list_t));
	pthread_t *thread;
	
	DIE(list == NULL, "can not alloc memmory!");
	list->compare_func = compare_thread;
	list->print_func = print_thread;
	list->free_func = free_thread;
	list->size = 0;
	list->head = NULL;

	return list;
}


void logmemcache_init_server_os(int *socket_server)
{
	int sock, client_size, client_sock;
	struct sockaddr_in server, client;
	list_t *list = (list_t *)malloc(sizeof(list_t));
	pthread_t *thread;
	DIE(list == NULL, "can not alloc memmory!");
	list->compare_func = compare_thread;
	list->print_func = print_thread;
	list->free_func = free_thread;
	list->size = 0;
	list->head = NULL;

	

	memset(&server, 0, sizeof(struct sockaddr_in));

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return;

	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
	server.sin_addr.s_addr = inet_addr(SERVER_IP);

	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Could not bind");
		exit(1);
	}

	if (listen(sock, DEFAULT_CLIENTS_NO) < 0) {
		perror("Error while listening");
		exit(1);
	}

	*socket_server = sock;

	while (1) {
		memset(&client, 0, sizeof(struct sockaddr_in));
		client_size = sizeof(struct sockaddr_in);
		client_sock = accept(sock, (struct sockaddr *)&client,
				(socklen_t *)&client_size);

		if (client_sock < 0) {
			perror("Error while accepting clients");
		}
		
		thread = malloc(sizeof(pthread_t));
		DIE (thread == NULL, "can not alloc memmory!");
		DIE(pthread_create(thread, NULL, start_client_thread, &client_sock) < 0, "failed to create a new thread");
	}

}

int logmemcache_init_client_cache(struct logmemcache_cache *cache)
{
	cache->pages_no = 0;
	cache->pages = create_page_list();
	return 0;
}

int logmemcache_add_log_os(struct logmemcache_client_st *client,
	struct client_logline *log)
{

	return 0;
}

int logmemcache_flush_os(struct logmemcache_client_st *client)
{

	return 0;
}

int logmemcache_unsubscribe_os(struct logmemcache_client_st *client)
{

	return 0;
}
int main() {
	list_t *list = (list_t *)malloc(sizeof(list_t));
	pthread_t *thread;
	DIE(list == NULL, "can not alloc memmory!");
	list->compare_func = compare_thread;
	list->print_func = print_thread;
	list->free_func = free_thread;
	list->size = 0;
	list->head = NULL;
	


	for (int i = 0; i < 100; i++) {
		thread = malloc(sizeof(pthread_t));
		DIE (thread == NULL, "can not alloc memmory!");
		pthread_create(thread, NULL, start_client_thread, &i);
		push_back(list, thread);
	}

	print_list(list);

	list_iterator_t it = get_list_it(list);
	pthread_t tid;
	while (has_next_list_it(it)) {
		pthread_join(*(pthread_t*)get_next_list_it(&it), NULL);

	}

	thread = get_last_element(list);

	print_thread(thread);
	free(it);
	free_list(list);
}