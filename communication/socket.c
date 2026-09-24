#include "socket.h"


typedef struct server_socket {

} server_socket;


server_socket* create_server_socket(PROTOCOL_FAMILY protocol, COMMUNICATION_TYPE type, int port){
	int sys_domain;
	int sys_type;

	switch (protocol){
		case TCP:
			sys_domain = AF_INET;
			break;
		case UDP:
			sys_domain = AF_INET6;
			break;
		default:
			return NULL;
	}
}
