#include "communication/socket.h"
#include "orchestrator/socket_management.h"
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void *test_server_socket(void *arg) {
	soc_server_socket *server_soc = soc_create_server_socket(SOC_TCP, SOC_IPV4, 8080);
	assert(server_soc != NULL);
	printf("Test1: Socket opened at port 8080\n");

	int listen_status = soc_start_listen(server_soc, 10);
	assert(listen_status >= 0);
	printf("Test1: start listening and await to accept a client socket\n");

	soc_client_socket *client_soc;
	int accept_status = soc_create_accept_client(server_soc, &client_soc);
	assert(accept_status > 0);
	printf("Test1: accepted a client socket\n");

	char *str = "Hello from server";
	size_t size = 18;
	int write_status = soc_write(client_soc, str, size);

	sleep(1);

	soc_client_socket_free(client_soc);
	soc_server_socket_free(server_soc);
	printf("Test1: server socket and client socket closed\n");
	return NULL;
}

void *test_client_socket_thread(void* arg) {
	soc_client_socket *client_soc;
	int create_socket_status = soc_create_connect_client_socket(&client_soc, SOC_TCP, SOC_IPV4, "127.0.0.1", 8080);
	assert(create_socket_status > 0);
	printf("Test2: connected to server socket\n");

	uint8_t b[18];
	soc_read(client_soc, &b, 18);

	char str[19];
	memcpy(str, b, 18);
	str[18] = '\0';
	printf("Test2: Received from server: %s\n", str);

	soc_client_socket_free(client_soc);
	printf("Test2: client socket is closed\n");
	return NULL;
}

void *test_orchestrator(void *args) {
	soc_server_socket *server_soc = soc_create_server_socket(SOC_TCP, SOC_IPV4, 8080);
	assert(server_soc != NULL);
	int listen_status = soc_start_listen(server_soc, 10);
	assert(listen_status >= 0);
	printf("Test3: server socket is initialized\n");
	soc_management_initiate_unverified_socket_queue();
	soc_management_initiate_verified_socket_queue();
	pthread_t acceptor_thread;
	pthread_create(&acceptor_thread, NULL, soc_management_acceptor, server_soc);
	printf("Test3: spind up acceptor\n");
	pthread_t handshake_thread;
	int timeout = 1;
	pthread_create(&handshake_thread, NULL, soc_management_handshake, &timeout);
	printf("Test3: spind up handshake\n");
	sleep(5);
}

void *test_client_socket_orchestrator(void *args) {
	soc_client_socket *soc;
	int create_status = soc_create_connect_client_socket(&soc, SOC_TCP, SOC_IPV4, "127.0.0.1", 8080);
	assert(create_status >= 0);
	printf("Test4: connected to orchestrator\n");
	char module_name[] = "test_module_1";
	char module_params[] = "a,b,c,d,e,f";
	soc_management_handshake_header handshake_header;
	handshake_header.module_name_length = 13;
	handshake_header.module_params_length = 11;
	soc_write(soc, &handshake_header, sizeof(handshake_header));
	printf("Test4: seent header to the sever");
	soc_write(soc, module_name, 13);
	soc_write(soc, module_params, 11);
	sleep(10);
}

int main(){
	printf("TEST #1: Bare Socket\n");
	pthread_t server_socket_thread;
	pthread_create(&server_socket_thread, NULL, test_server_socket, NULL);
	sleep(1);
	pthread_t client_socket_thread;
	pthread_create(&client_socket_thread, NULL, test_client_socket_thread, NULL);

	pthread_join(server_socket_thread, NULL);
	pthread_join(client_socket_thread, NULL);

	sleep(1);
	printf("\nTEST #2: Orchestrator\n");

	pthread_t orchestrator_thread;
	pthread_create(&orchestrator_thread, NULL, test_orchestrator, NULL);
	sleep(1);
	pthread_t client_orchestrator_thread;
	pthread_create(&client_orchestrator_thread, NULL, test_client_socket_orchestrator, NULL);
	pthread_join(orchestrator_thread, NULL);
	pthread_join(client_orchestrator_thread, NULL);
	return 1;
}
