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
 * @brief accept an incoming connection.
 * call to this functions will be made outside of this file. opaque ptr will prevent it initiate soc_client_socket.
 * passing double pointer to point to the malloc ptr.
 * MALLOC IS CALLED, MUST BE FREE AFTERWARD.
 *
 * @param soc_server_socket instance of server_socket contains socket_df
 * @param soc_client_socket pointer-pointer-soc_client_socket
 *
 * @return
 * 1 if the process is sucess
 * -1 is the params fail the check
 * -2 if fail to create file descriptor for client socket
 * -3 if fail to malloc soc_client_socket
 */
int soc_create_accept_client(soc_server_socket *, soc_client_socket **);

/**
 * @brief write raw bytes into socket file descriptor
 *
 * @param client_socket ~ struct contains socket_df
 * @param data ~ data to send
 * @param data_len ~ number of byte of the data
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
 * @return 1 if sucessfully read from the socket
 * -1 if the parametter check isn't qualified
 * -2 if socket is closed
 */
int soc_read(const soc_client_socket *, void *, size_t);

/**
 * @brief
 * free the client socket.
 *
 * @param client_soc *
 * ptr to client socket
 */
void soc_client_socket_free(soc_client_socket *);


/**
 * @breif
 * free the server socket
 *
 * @param soc_server_socket
 *
 * @param server_soc *
 * ptr to server socket
 */
void soc_server_socket_free(soc_server_socket *);

/**
 * @brief
 * get the socket file descriptor. due to the opaque is too good.
 *
 * @param soc - socket struct to extract socket file descriptor
 *
 * @return socket descriptor
 * -1 if the socket struct is NULL
 */
int soc_get_client_fd(const soc_client_socket *);

/**
 * @brief
 * create a client socket to a given host
 *
 * @param soc
 * - instance of client socket to malloc
 * @param soc_protocol
 * - TCP / UDP type
 * @param soc_addr
 * - IpV4 / IPv6 type
 * @param arr
 * - IP address of the host
 * @param port
 * - port to connect of the host
 *
 * @return 1 if nothing wrong
 * -1 when params check fail
 * -2 when fail to create socket_df
 * -3 when fail to convert string IP addr to binary
 * -4 when fail to TCP handshake
 * -5 when fail to malloc soc_client_socket
 */
int soc_create_connect_client_socket(soc_client_socket **, SOC_PROTOCOL_FAMILY, SOC_ADDRESS_FAMILY, const char *, int);
#endif
