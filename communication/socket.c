#include "socket.h"
#include <stdint.h>
#include <sys/socket.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>


struct soc_server_socket {
	int socket_df;
	int port;
	SOC_PROTOCOL_FAMILY protocol_type;
	SOC_ADDRESS_FAMILY address_type;
	int max_queue;
};


struct soc_client_socket {
	int socket_df;
	SOC_ADDRESS_FAMILY address_type;
};


soc_server_socket *soc_create_server_socket(SOC_PROTOCOL_FAMILY protocol_type, SOC_ADDRESS_FAMILY address_type, int port){
	int sys_protocol_type;
	int sys_family_type;

	switch (protocol_type) {
		case SOC_TCP:
			sys_protocol_type = SOCK_STREAM;
			break;
		case SOC_UDP:
			sys_protocol_type = SOCK_DGRAM;
			break;
		default:
			return NULL;
	}

	switch (address_type) {
		case SOC_IPV4:
			sys_family_type = AF_INET;
			break;
		case SOC_IPV6:
			sys_family_type = AF_INET6;
			break;
		default:
			// not supported yet
			return NULL;
	}

	// 1. create socket - the receptionist
	int socket_df = socket(sys_family_type, sys_protocol_type, 0);
	if (socket_df < 0){
		return  NULL;
	}

	// 2. bind the above socket to a port - give a position where receptionist greet incoming people
	struct sockaddr *addr = NULL;
	struct sockaddr_in server_addr;
	struct sockaddr_in6 server_addr6;
	socklen_t addr_len = 0;
	switch (address_type) {
		case SOC_IPV4:
			memset(&server_addr, 0, sizeof(server_addr));
			server_addr.sin_family = sys_family_type;
			server_addr.sin_port = htons(port);
			server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			addr = (struct sockaddr*) &server_addr;
			addr_len = sizeof(server_addr);
			break;
		case SOC_IPV6:
			memset(&server_addr6, 0, sizeof(server_addr6));
			server_addr6.sin6_family = sys_family_type;
			server_addr6.sin6_port = htons(port);
			server_addr6.sin6_addr = in6addr_any;
			addr = (struct sockaddr*) &server_addr6;
			addr_len = sizeof(server_addr6);
			break;
		default:
			// not supported yet
			close(socket_df);
			return NULL;
	}

	int bind_result = bind(socket_df, addr, addr_len);
	if (bind_result < 0) {
	    close(socket_df);
		return NULL;
	}

	soc_server_socket *new_socket = malloc(sizeof(soc_server_socket));
	if (new_socket == NULL) {
		close(socket_df);
		return NULL;
	}
	new_socket -> socket_df = socket_df;
	new_socket -> port = port;
	new_socket -> protocol_type = protocol_type;
	new_socket -> address_type = address_type;
	new_socket -> max_queue = -1;
	return new_socket;
}


int soc_start_listen(soc_server_socket *soc, int max_queue){
	if (soc == NULL || soc -> protocol_type != SOC_TCP){
		return -1;
	}
	int listen_status = listen(soc -> socket_df, max_queue);
	if (listen_status < 0){
		return -1;
	}
	soc -> max_queue = max_queue;
	return listen_status;
}


soc_client_socket *soc_accept_client(soc_server_socket *server_soc){
	if (server_soc == NULL || server_soc -> protocol_type != SOC_TCP){
		return NULL;
	}

	struct sockaddr_storage client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t soc_len = sizeof(client_addr);
	int soc_df = accept(server_soc -> socket_df, (struct sockaddr*) &client_addr, &soc_len);
	if (soc_df < 0){
		return NULL;
	}

	soc_client_socket *client_soc = malloc(sizeof(soc_client_socket));
	if (client_soc == NULL) {
		close(soc_df);
		return NULL;
	}
	client_soc -> socket_df = soc_df;
	client_soc -> address_type = server_soc -> address_type;

	return client_soc;
}

int soc_write(const soc_client_socket *soc, const void* data, size_t data_len) {
	if (soc == NULL || data == NULL || data_len == 0){
		return -1;
	}

	const uint8_t *byte_ptr = (const uint8_t *) data;
	size_t sent_bytes = 0;

	while (sent_bytes < data_len){
		ssize_t res = write(soc -> socket_df, byte_ptr + sent_bytes, data_len - sent_bytes);
		if (res <= 0 ) {
			return -1;
		}
		sent_bytes += res;
	}

	return 1;
}

int soc_read(const soc_client_socket *soc, void *buffer, size_t data_len) {
	if (soc == NULL || buffer == NULL || data_len == 0) {
		return -2;
	}
	uint8_t *buffer_byte_ptr = (uint8_t *) buffer;
	size_t read_bytes = 0;

	while (read_bytes < data_len){
		ssize_t res = read (soc -> socket_df, buffer_byte_ptr + read_bytes, data_len - read_bytes);
		if (res <= 0){
			return -1;
		}
		read_bytes += res;
	}

	return 1;
}
