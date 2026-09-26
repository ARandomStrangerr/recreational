#include "socket.h"


struct server_socket {
	int socket_df;
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
	
	int socket_df = socket(sys_address_type, sys_protocol_type, 0);
	if (socket_df < 0){
		return  NULL;
	}

	server_socket *socket = malloc(sizeof(server_socket));
	socket -> socket_df = socket_df;

	return socket;
}
