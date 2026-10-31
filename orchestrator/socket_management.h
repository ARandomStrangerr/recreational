#ifndef SOC_MANAGEMENT
#define SOC_MANAGEMENT


#include <stdint.h>


typedef struct __attribute__((__packed__)) {
    uint32_t correlation_id;
    uint32_t target_service_size;
    uint32_t payload_size;
} soc_management_header;


/**
 * @brief
 * initialize the unverified socket queue.
 * depends on the operating system, macOS will initiate `kqueue`, linux will initiate `epoll`
 * must be called before soc_management_acceptor() or soc_management_handshake_checker()
 *
 * @return
 * 1 if sucess to allocate
 * -1 if failure to allocate
 */
int soc_management_initiate_unverified_socket_queue();

/**
 * @brief
 * initialize the verified socket queue.
 * depends on the operating system, macOS will initiate `kqueue`, linux will initiate `epoll`
 * must be called before soc_management_acceptor() or soc_management_handshake_checker()
 *
 * @return
 * 1 if sucess to allocate
 * -1 if failure to allocate
 */
int soc_management_initiate_verified_socket_queue();

/**
 * @brief
 * function complies with thread singnature.
 * this function handle the handshake of unregistered sockets.
 * will forcefully wakeup every 1s to check.
 * whatever the socket fail or not complete the hanshake within fixed amount of time will be drop.
 *
 * @param arg amount of time to complete the handshake
 */
void *soc_management_handshake(void *);

/**
 * @brief
 * function complies with thread signature.
 * this function has an infinite loop to accept incoming socket then stash it into a stack for further handshake.
 * if server_socket passed in as null, immediate terminate.
 * if soc_management_initiate_unverified_socket_queue has not been called, immediate terminate.
 *
 * @param arg soc_server_socket ptr instance
 */
void *soc_management_acceptor(void *);

/**
 * @brief
 * function complies with thread signature.
 * this function will handle handshake from unregistered sockets.
 */
void *soc_management_handshake(void *);

/**
 * @brief
 * function complies with thread signature.
 * this function will handle request from registered sockets.
 */
void *soc_management_request_hander(void *);

/**
 * @brief
 * function complies with thread signature.
 * this function listen to verified sockets and have a thread handle the request.
 */
void *soc_management_router(void *);

#endif
