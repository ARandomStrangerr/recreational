#include "socket_management.h"
#include "../communication/socket.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/event.h>
#include "../structure/uthash.h"

#define MAX_EVENTS 64


typedef struct{
	soc_client_socket *soc;
	char *module_name;
	char **module_params;
	uint8_t *msg;
	size_t msg_size;
	size_t received;
	UT_hash_handle hh;
} soc_management_socket_entry;


static int unverified_queue_fd = -1;
static int verified_queue_fd = -1;
static soc_client_socket *client_socket_tbl[8];
static soc_management_socket_entry *service_map = NULL;


int soc_management_initiate_unverified_socket_queue(){
	if (unverified_queue_fd == -1) unverified_queue_fd = kqueue();
	return unverified_queue_fd;
}


int soc_management_initiate_verified_socket_queue() {
	if (verified_queue_fd == -1) verified_queue_fd = kqueue();
	return verified_queue_fd;
}


/**
 * @bref
 * helper function to put a socket into kqueue
 *
 * @param *soc socket to put in unverified queue
 * @parma queue_df ~ queue to put the socket into
 *
 * @return
 * 1 if the acion is complete
 * -1 if unverified queue was not initiated
 * -2 if the socket is null
 * -3 if the fail to get socket_df
 * -4 if fail to push the socket into queue
 */
static int soc_management_register_watch_socket(const soc_client_socket *soc, int queue_fd){
	if (queue_fd == -1) return -1;
	if (soc == NULL) return -2;

	int soc_fd = soc_get_client_fd(soc);
	if (soc_fd < 0) return -3;

	struct kevent change_event;
	EV_SET(&change_event, soc_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void *)soc);
	int push_status = kevent(queue_fd, &change_event, 1, NULL, 0, NULL);
	if (push_status < 0) return -4;
	return 1;
}


/**
 * @brief
 * helper function to remove a socket from kqueue
 *
 * @param *soc ~ socket to put into queue
 * @param queue ~ queue to remove socket from
 *
 * @return 1 ~ if nothing wrong
 * -1 ~ params check fail
 * -2 ~ fail to get client socket
 * -3 ~ fail to remove socket from queue
 */
static int soc_management_withdraw_watch_socket(const soc_client_socket *soc, int queue_df){
	if (queue_df == -1 || soc == NULL) return -1;

	int soc_fd = soc_get_client_fd(soc);
	if (soc_fd < 0) return -2;

	struct kevent delete_event;
	EV_SET(&delete_event, soc_fd, EVFILT_READ, EV_DELETE, 0, 0, (void *)soc);
	int delete_status = kevent(queue_df, &delete_event, 1, NULL, 0, NULL);
	if (delete_status < 0) return -3;

	return 1;
}


void *soc_management_handshake(void *arg){
	if (unverified_queue_fd == -1 || verified_queue_fd == -1) {
		return NULL;
	}

	int amount_timeout = *((int *) arg); // convert to integer

	struct kevent active_events[MAX_EVENTS]; // array to hold the viable sockets

	struct timespec timeout; // set timeout
	timeout.tv_sec = amount_timeout; // seconds
	timeout.tv_nsec = 0; // nanoseconds

	while(1){
		int ready_count = kevent(unverified_queue_fd, NULL, 0, active_events, MAX_EVENTS, &timeout); // wait for maximum `timeout`, then get a list of sockets that have message ready

		for (int i = 0; i < ready_count; i++) { // itereate through ready sockets
			soc_client_socket *soc = (soc_client_socket *) active_events[i].udata; // ready socket

			if (soc == NULL) continue; // defends against ghost pointer

			if (active_events[i].flags & EV_EOF) { // check if the socket is closed
				soc_client_socket_free(soc);
				soc_management_withdraw_watch_socket(soc, unverified_queue_fd);
				continue;
			}

			// handshake procedure
			// todo: non-block read from socket
			soc_management_handshake_header handshake_header;
			soc_read(soc, &handshake_header, sizeof(handshake_header));
			// todo: check if the reading is success
			// todo: check the size of the module_name_length and module_param_length
			char module_name[handshake_header.module_name_length + 1];
			soc_read(soc, module_name, handshake_header.module_name_length);
			// todo: check if the reading is success
			module_name[handshake_header.module_name_length] = '\0';
			char module_params[handshake_header.module_params_length + 1];
			soc_read(soc, module_params, handshake_header.module_params_length);
			// todo: check if the reading is success
			module_params[handshake_header.module_params_length] = '\0';

			soc_management_socket_entry *entry = malloc(sizeof(soc_management_socket_entry));
			entry -> module_name = malloc(strlen(module_name) + 1);
			strcpy(entry->module_name, module_name);
			entry -> module_name[strlen(module_name)] = '\0';
			entry -> soc = soc;

			HASH_ADD_STR(service_map, module_name, entry);
			soc_management_register_watch_socket(soc, verified_queue_fd);
			soc_management_withdraw_watch_socket(soc, unverified_queue_fd);
		}
	}
	return NULL;
}


void *soc_management_request_handler(void* arg) {
	if (verified_queue_fd == -1) return NULL;

	struct kevent active_events[MAX_EVENTS];

	while(1) {
		int ready_count = kevent(verified_queue_fd, NULL, 0, active_events, MAX_EVENTS, NULL);

		for (int i = 0; i < ready_count; i++) {
			soc_client_socket *soc = (soc_client_socket *) active_events[i].udata;

			size_t msg_size;
			soc_read(soc, &msg_size, sizeof(msg_size));

			char module_name[msg_size + 1];
			soc_read(soc, module_name, msg_size);
			module_name[msg_size] = '\0';

			soc_read(soc, &msg_size, sizeof(msg_size));

			char data[msg_size + 1];
			soc_read(soc, data, msg_size);
			data[msg_size] = '\0';

			soc_management_socket_entry *entry;

			HASH_FIND_STR(service_map, module_name, entry);
			printf("retreated socket\n");
			soc_write(entry->soc, &msg_size, sizeof(msg_size));
			soc_write(entry->soc, data, msg_size);
		}
	}
	return NULL;
}


void *soc_management_router(void *arg) {
	if (verified_queue_fd == -1) return NULL;
}

void *soc_management_acceptor(void *arg){
	if (arg == NULL || unverified_queue_fd == -1) {
		return NULL;
	}

	soc_server_socket *server_soc = (soc_server_socket *) arg;
	soc_client_socket *client_soc = NULL;
	int status = 0;

	while (1){
		status = soc_create_accept_client(server_soc, &client_soc);
		if (status == 1){
			int push_status = soc_management_register_watch_socket(client_soc, unverified_queue_fd); // register the newly accepted socket with unverified queue
			if (push_status < 0) {
				printf("WARNING: unverified queue full or error, drop the socket");
				soc_client_socket_free(client_soc);
			}
		} else if (status == -1) {
			printf("FATAL: incorrect params\n");
			break;
		} else if (status == -2) {
			printf("ERROR: fail to create client socket FD\n");
			continue;
		} else if (status == -3) {
			printf("FATAL: fail to malloc soc_client_socket\n");
			break;
		}
	}
	return NULL;
}
