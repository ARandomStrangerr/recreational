#include "socket.h"


struct server_socket {
	int socket_df;
	int port;
};


server_socket *create_server_socket(PROTOCOL_FAMILY protocol_type, ADDRESS_FAMILY address_type, int port){
	int sys_address_type;
	int sys_protocol_type;
	switch (address_type) {
		case IPV4:
			sys_address_type = AF_INET;
			break;
		case IPV6:
			sys_address_type = AF_INET6;
			break;
		case LOCAL_HOST:
			sys_address_type = AF_LOCAL;
			break;
	}

	switch (protocol_type) {
		case TCP:
			sys_protocol_type = SOCK_STREAM;
			break;
		case UDP:
			sys_protocol_type = SOCK_DGRAM;
			break;
	}
	
	// 1. create socket - the receptionist
	int socket_df = socket(sys_address_type, sys_protocol_type, 0);
	if (socket_df < 0){
		return  NULL;
	}
	
	// 2. bind the above socket to a port - give a position where receptionist greet incoming people
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = sys_address_type;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	int bind_result = bind(socket_df, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (bind_result < 0) {
		return NULL;
	}

	server_socket *socket = malloc(sizeof(server_socket));
	if (socket == NULL) {
		close(socket_df);
		return NULL;
	}
	socket -> socket_df = socket_df;
	socket -> port = port;

	return socket;
}
