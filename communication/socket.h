#ifndef SOCKET_H
#define SOCKET_H


#include <unistd.h>


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
 * @param SOC_PROTOCOL_FAMILY choosing between TCP vs UDP
 * @param SOC_ADDRESS_FAMILY choosing IPv4, IPv6, or Local
 * @param int port to bind to
 * @return instant of server_socket that is binded to given port, NULL if the code fail
 */
soc_server_socket *soc_create_server_socket(SOC_PROTOCOL_FAMILY, SOC_ADDRESS_FAMILY, int);


/**
 * @brief start listening to incoming connection
 *
 * @param *soc declared socket must be TCP
 * @param max_queue how may incoming connection can queue at the same time
 * @return negative number if fail to initiate the listening process
 */
int soc_start_listen(soc_server_socket*, int);

/**
 * @brief accept an incoming connection
 *
 * @param soc_server_soc instance of server_socket contains socket_df
 */
soc_client_socket *soc_accept_client(soc_server_socket *);

/**
 * @brief write raw bytes into socket file descriptor
 *
 * @param client_socket struct contains socket_df
 * @param data data to send
 * @param data_len number of byte of the data
 * @return 1 if the writing is completed -1 if there is error when we send the bytes
 */
int soc_write(const soc_client_socket *, const void *, size_t);

/**
 * @brief read raw bytes from socket file descriptor.
 * return int because if fail to read return NULL could be anything: socket closed, other socket send null
 *
 * @param soc_client_socket struct contains socket_df
 * @param buffer to write on
 * @param read_byte number of byte to read
 * @return 1 if sucessfully read from the socket, -1 if socket is closed, -2 if the parametter check isn't qualified
 */
int soc_read(const soc_client_socket *, void *, size_t);

#endif
