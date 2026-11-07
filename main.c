#include "communication/socket.h"
#include "orchestrator/socket_management.h"
#include <assert.h>
#include <stddef.h>
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
	pthread_t request_hander_thread;
	pthread_create(&request_hander_thread, NULL, soc_management_request_handler, NULL);
	sleep(5);
	return NULL;
}

void *test_module_1(void *args) {
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
	printf("Test4: send header to the orchestrator\n");
	soc_write(soc, module_name, 13);
	soc_write(soc, module_params, 11);
	printf("Test4: send module name and params to the orchestrator\n");
	sleep(4);
	char receive_module_name[] = "test_module_2";
	size_t receive_module_name_size = strlen(receive_module_name);
	char data[] = "hello 1234 from module 1";
	size_t data_size = strlen(data);
	soc_write(soc, &receive_module_name_size, sizeof(size_t));
	soc_write(soc, receive_module_name, receive_module_name_size);
	soc_write(soc, &data_size, sizeof(size_t));
	soc_write(soc, data, data_size);
	sleep(10);
	return NULL;
}

void *test_moduele_2 (void *arg) {
	soc_client_socket *soc;
	int create_status = soc_create_connect_client_socket(&soc, SOC_TCP, SOC_IPV4, "127.0.0.1", 8080);
	assert(create_status >= 0);
	printf("Test5: connected to orchestrator\n");
	char module_name[] = "test_module_2";
	char module_params[] = "a,b,c,d,e,f";
	soc_management_handshake_header handshake_header;
	handshake_header.module_name_length = 13;
	handshake_header.module_params_length = 11;
	soc_write(soc, &handshake_header, sizeof(handshake_header));
	printf("Test5: send header to the orchestartor\n");
	soc_write(soc, module_name, 13);
	soc_write(soc, module_params, 11);
	printf("Test5: send module name and params to the orchestrator\n");
	size_t incoming_data_len;
	soc_read(soc, &incoming_data_len, sizeof(incoming_data_len));
	char data[incoming_data_len + 1];
	soc_read(soc, data, incoming_data_len);
	data[incoming_data_len] = '\0';
	printf("%s\n", data);

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
	pthread_t module_1;
	pthread_create(&module_1, NULL, test_module_1, NULL);
	pthread_t module_2;
	pthread_create(&module_2, NULL, test_moduele_2, NULL);
	pthread_join(module_1, NULL);
	pthread_join(module_2, NULL);
	pthread_join(orchestrator_thread, NULL);
	return 1;
};
