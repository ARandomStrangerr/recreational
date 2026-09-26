#include <sys/socket.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

typedef enum {
	IPV4,
	IPV6,
	LOCAL_HOST
} ADDRESS_FAMILY;


typedef enum {
	TCP,
	UDP
} PROTOCOL_FAMILY;


typedef struct server_socket server_socket;


typedef struct client_socket client_socket;


/**
 * @bref helper function to make a socket and bind to a port
 *
 * @param	COMMUNICATION_TYPE choosing IPv4, IPv6, or Local
 * @param PROTOCOL_FAMILY choosing between TCP vs UDP
 * @param int port to bind to
 */
server_socket *create_server_socket(PROTOCOL_FAMILY, ADDRESS_FAMILY, int);
