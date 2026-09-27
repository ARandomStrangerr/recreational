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
 * @brief helper function to make a socket and bind to a port
 *
 * @param	COMMUNICATION_TYPE choosing IPv4, IPv6, or Local
 * @param PROTOCOL_FAMILY choosing between TCP vs UDP
 * @param int port to bind to
 * @return instant of server_socket that is binded to given port, NULL if the code fail
 */
server_socket *create_server_socket(PROTOCOL_FAMILY, ADDRESS_FAMILY, int);


/**
 * @brief start listening to incoming connection
 *
 * @params *soc declared socket must be TCP
 * @params max_queue how may incoming connection can queue at the same time
 * @return negative number if fail to initiate the listening process
 */
int start_listen(server_socket*, int);

/**
 * @brief accept an incoming connection
 *
 * @params server_soc instance of server_socket contains socket_df
 */
client_socket *accept_client(server_socket *);
