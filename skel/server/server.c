/**
 * Hackathon SO: LogMemCacher
 * (c) 2020-2021, Operating Systems
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../include/server.h"

#ifdef __unix__
#include <sys/socket.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

static struct logmemcache_cache **caches;
static size_t cache_count;
static size_t max_caches;

static SOCKET server_socket;

void print_page(void *data) {
	page_t *page = (page_t*)data;

	printf("address=%p is_flushed=%c current_index=%ld", page->addr, page->is_flushed, page->current_index);
}

char compare_page(void *data_1, void *data_2) {
	page_t *page_1 = (page_t *)data_1;
	page_t *page_2 = (page_t *)data_2;

	return (page_1->addr == page_2->addr);
}

void free_page(void *data) {
	page_t* page = (page_t *)data;

	free(page);
}

list_t* create_page_list() {
	list_t *list = (list_t *)malloc(sizeof(list_t));
	pthread_t *thread;

	DIE(list == NULL, "can not alloc memmory!");
	list->compare_func = compare_page;
	list->print_func = print_page;
	list->free_func = free_page;
	list->size = 0;
	list->head = NULL;

	return list;
}

/* Server API */
static void init_client_list(void)
{

	max_caches = DEFAULT_CLIENTS_NO;
	caches = malloc(max_caches * sizeof(*caches));
}

static void init_server(void)
{

	init_client_list();
	logmemcache_init_server_os(&server_socket);
}

struct logmemcache_client_st *logmemcache_create_client(SOCKET client_sock)
{
	struct logmemcache_client_st *client;

	client = malloc(sizeof(*client));
	client->client_sock = client_sock;
	client->cache = NULL;

	return client;
}

static int logmemcache_add_client(struct logmemcache_client_st *client,
	char *name)
{
	int err = 0;
	size_t i;

	for (i = 0; i < cache_count; i++) {
		if (caches[i] == NULL)
			continue;
		if (caches[i]->service_name == NULL)
			continue;
		if (strcmp(caches[i]->service_name, name) == 0) {
			client->cache = caches[i];
			goto found;
		}
	}

	if (cache_count == max_caches) {
		return -1;
	}

	client->cache = malloc(sizeof(*client->cache));
	client->cache->service_name = strdup(name);
	caches[cache_count++] = client->cache;

	err = logmemcache_init_client_cache(client->cache);
found:
	return err;
}

static int logmemcache_disconnect_client(struct logmemcache_client_st *client)
{

	return 0;
}

static int logmemcache_unsubscribe_client(struct logmemcache_client_st *client)
{

	return 0;
}

static int logmemcache_add_log(struct logmemcache_client_st *client,
 struct client_logline *log)
{
 size_t logsize;
 page_t *page = get_last_element(client->cache->pages);
 write_to_page(page, log->logline, client->cache->pages);
 
 return 0;
}

static int logmemcache_flush(struct logmemcache_client_st *client)
{

	return 0;
}

static int logmemcache_send_stats(struct logmemcache_client_st *client)
{

	return 0;
}

static int logmemcache_send_loglines(struct logmemcache_client_st *client)
{

	return 0;
}

static void parse_command(struct command *cmd, char *string, ssize_t *datalen)
{
	char *command, *line;

	command = strdup(string);
	line = strchr(command, ' ');

	cmd->data = NULL;
	if (line != NULL) {
		line[0] = '\0';
		cmd->data = strdup(line + 1);
		*datalen -= strlen(command) + 1;
	}

	cmd->op = get_op_by_str(command);

	printf("command = %s, line = %s\n", cmd->op->op_str,
			cmd->data ? cmd->data : "null");

	free(command);
}

static int validate_arg(const char *line, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
		if (!isprint(line[i]))
			return -1;

	return 0;
}

int get_command(struct logmemcache_client_st *client)
{
	int err;
	ssize_t recv_size;
	char buffer[COMMAND_SIZE], response[LINE_SIZE];
	char *reply_msg;
	struct command cmd;
	struct client_logline *log;

	err = -1;

	memset(&cmd, 0, sizeof(cmd));
	memset(buffer, 0, sizeof(buffer));
	memset(response, 0, sizeof(response));

	recv_size = recv_data(client->client_sock, buffer, sizeof(buffer), 0);
	if (recv_size <= 0)
		return -1;

	parse_command(&cmd, buffer, &recv_size);
	if (recv_size > LINE_SIZE) {
		reply_msg = "message too long";
		goto end;
	}

	if (cmd.op->requires_auth && client->cache->service_name == NULL) {
		reply_msg = "authentication required";
		goto end;
	}

	if (cmd.data != NULL) {
		err = validate_arg(cmd.data, recv_size);
		if (err != 0) {
			reply_msg = "invalid argument provided";
			goto end;
		}
	}

	switch (cmd.op->code) {
	case CONNECT:
	case SUBSCRIBE:
		err = logmemcache_add_client(client, cmd.data);
		break;
	case STAT:
		err = logmemcache_send_stats(client);
		break;
	case ADD:
		log = NULL;
		err = logmemcache_add_log(client, log);
		break;
	case FLUSH:
		err = logmemcache_flush(client);
		break;
	case DISCONNECT:
		err = logmemcache_disconnect_client(client);
		break;
	case UNSUBSCRIBE:
		err = logmemcache_unsubscribe_client(client);
		break;
	case GETLOGS:
		err = logmemcache_send_loglines(client);
		break;
	default:
		/* unknown command */
		err = -1;
		break;
	}

	reply_msg = cmd.op->op_reply;

end:
	if (err == 0)
		sprintf(response, "%s", reply_msg);
	else
		sprintf(response, "FAILED: %s", reply_msg);

	if (cmd.data != NULL)
		free(cmd.data);

	err = send_data(client->client_sock, response, LINE_SIZE, SEND_FLAGS);
	if (err < 0)
		return -1;

	return err;
}

page_t *init_new_page(char *addr)
{
 page_t *new_page = malloc(sizeof(page_t));
 new_page->addr = addr;
 new_page->current_index = 0;
 new_page->is_flushed = 0;
 
 return new_page;
}
 
void write_to_page(page_t *page, char *data, list_t *page_list)
{
	char *mem;
	page_t *new_page = NULL;
	size_t pagesize = getpagesize(), copied;
	size_t data_size = strlen(data);
	
	if (pagesize - page->current_index >= data_size)
	{
		memcpy(page->addr + page->current_index, data, data_size);
		page->current_index += data_size;
	}
	else
	{
		copied = pagesize - page->current_index;
		memcpy(page->addr + page->current_index, data, copied);
		page->current_index = pagesize;
		mem = mmap(NULL, pagesize, PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANON, -1, 0);
		DIE(mem == MAP_FAILED, "failed to map memory!");
		
		new_page = init_new_page(mem);
		memcpy(mem, data + copied, data_size - copied);
		push_back(page_list, new_page);
	}
}
// int main(int argc, char *argv[])
// {
// 	setbuf(stdout, NULL);

// 	if (argc == 1)
// 		logfile_path = strdup("logs_logmemcache");
// 	else
// 		logfile_path = strdup(argv[1]);

// 	if (init_logdir(logfile_path) < 0)
// 		exit(-1);

// 	init_server();

// 	return 0;
// }
