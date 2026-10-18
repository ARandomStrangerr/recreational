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
	printf("Socket opened at port 8080\n");

	int listen_status = soc_start_listen(server_soc, 10);
	assert(listen_status >= 0);
	printf("start listening and await to accept a client socket\n");

	soc_client_socket *client_soc;
	int accept_status = soc_create_accept_client(server_soc, &client_soc);
	printf("%d\n", accept_status);
	assert(accept_status > 0);
	printf("accepted a client socket");

	char *str = "Hello from server";
	size_t size = 18;
	int write_status = soc_write(client_soc, str, size);

	return NULL;
}

void *test_client_socket_thread(void* arg) {
	soc_client_socket *client_soc;
	int create_socket_status = soc_create_connect_client_socket(&client_soc, SOC_TCP, SOC_IPV4, "127.0.0.1", 8080);
	assert(create_socket_status > 0);
	printf("connected to server socket");

	uint8_t b[18];
	soc_read(client_soc, &b, 18);

	char str[19];
	memcpy(str, b, 18);
	str[18] = '\0';
	printf("Received from server: %s/n", str);

	return NULL;
}

int main(){
	pthread_t server_socket_thread;
	pthread_create(&server_socket_thread, NULL, test_server_socket, NULL);
	sleep(1);
	pthread_t client_socket_thread;
	pthread_create(&client_socket_thread, NULL, test_client_socket_thread, NULL);

	pthread_join(server_socket_thread, NULL);
    pthread_join(client_socket_thread, NULL);
	return 1;
}
