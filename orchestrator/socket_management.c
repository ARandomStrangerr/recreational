#include "socket_management.h"
#include "../communication/socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>

#define MAX_EVENTS 64


static int unverified_queue_fd = -1;
static int verified_queue_fd = -1;


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
 * helper function to intiate unverified queue for socket
 *
 * @param soc socket to put in unverified queue
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
				continue;
			}

			soc_management_header header;
			int read_status = soc_read(soc, &header, sizeof(header));
			if (read_status < 0) { // case when fail to read from socket
				continue;
			}
		}
	}
	return NULL;
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
