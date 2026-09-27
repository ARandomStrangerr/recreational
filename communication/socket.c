#include "socket.h"
#include <netinet/in.h>


struct server_socket {
	int socket_df;
	int port;
	PROTOCOL_FAMILY protocol_type;
	ADDRESS_FAMILY address_type;
};


server_socket *create_server_socket(PROTOCOL_FAMILY protocol_type, ADDRESS_FAMILY address_type, int port){
	int sys_protocol_type;
	int sys_family_type;

	switch (protocol_type) {
		case TCP:
			sys_protocol_type = SOCK_STREAM;
			break;
		case UDP:
			sys_protocol_type = SOCK_DGRAM;
			break;
	}

	switch (address_type) {
		case IPV4:
			sys_family_type = AF_INET;
			break;
		case IPV6:
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
		case IPV4:
			memset(&server_addr, 0, sizeof(server_addr));
			server_addr.sin_family = sys_family_type;
			server_addr.sin_port = htons(port);
			server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			addr = (struct sockaddr*) &server_addr;
			addr_len = sizeof(server_addr);
			break;
		case IPV6:
			memset(&server_addr6, 0, sizeof(server_addr6));
			server_addr6.sin6_family = sys_family_type;
			server_addr6.sin6_port = htons(port);
			server_addr6.sin6_addr = in6addr_any;
			addr = (struct sockaddr*) &server_addr6;
			addr_len = sizeof(server_addr6);
			break;
		default:
		    // not supported yet
			return NULL;
	}

	int bind_result = bind(socket_df, addr, addr_len);
	if (bind_result < 0) {
	    close(socket_df);
		return NULL;
	}

	server_socket *new_socket = malloc(sizeof(server_socket));
	if (new_socket == NULL) {
		close(socket_df);
		return NULL;
	}
	new_socket -> socket_df = socket_df;
	new_socket -> port = port;
	new_socket -> protocol_type = protocol_type;
	new_socket -> address_type = address_type;

	return new_socket;
}
