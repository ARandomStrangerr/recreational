#ifndef SOCKET_H
#define SOCKET_H


typedef enum {
	SOC_IPV4,
	SOC_IPV6,
	SOC_LOCAL_HOST
} SOC_ADDRESS_FAMILY;


typedef enum {
	SOC_TCP,
	SOC_UDP
} SOC_PROTOCOL_FAMILY;


typedef struct soc_server_socket soc_server_socket;


typedef struct soc_client_socket soc_client_socket;


/**
 * @brief helper function to make a socket and bind to a port
 *
 * @param	COMMUNICATION_TYPE choosing IPv4, IPv6, or Local
 * @param PROTOCOL_FAMILY choosing between TCP vs UDP
 * @param int port to bind to
 * @return instant of server_socket that is binded to given port, NULL if the code fail
 */
soc_server_socket *soc_create_server_socket(SOC_PROTOCOL_FAMILY, SOC_ADDRESS_FAMILY, int);


/**
 * @brief start listening to incoming connection
 *
 * @params *soc declared socket must be TCP
 * @params max_queue how may incoming connection can queue at the same time
 * @return negative number if fail to initiate the listening process
 */
int soc_start_listen(soc_server_socket*, int);

/**
 * @brief accept an incoming connection
 *
 * @params server_soc instance of server_socket contains socket_df
 */
soc_client_socket *soc_accept_client(soc_server_socket *);

/**
 * @brief write a string into socket file descriptor
 *
 * @params client_socket struct contains socket_df
 * @params str a string to send to client
 */
int soc_write(soc_client_socket *, const char *);

/**
 * @brief read a string from socket file descriptor
 *
 * @params client_socket struct contains socket_df
 * @returns a string
 */
char *soc_read(soc_client_socket *);

#endif
