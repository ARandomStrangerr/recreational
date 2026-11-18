#include "log.h"
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#define MSG_LOG_SIZE 512

static char log_msg_stogate[MSG_LOG_SIZE];
static size_t log_msg_start_pos = 0;
static size_t log_msg_end_pos = 0;
static pthread_mutex_t log_msg_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t log_msg_read_cond = PTHREAD_COND_INITIALIZER;


/**
 * idea is to use circular array as log to hold message.
 */

void *log_init(void *arg){
	while (1) {
		pthread_cond_wait(&log_msg_read_cond, &log_msg_mutex);
		while(log_msg_start_pos != log_msg_end_pos) {

		}
		pthread_mutex_unlock(&log_msg_mutex);
	}
}

int log_write(log_level level, const char *msg, ...) {
	// ----- check params -----
	if (msg == NULL) return -1; // when no message passed

	// ----- create the string to be print out -----
	char log_msg[9 + strlen(msg)];
	log_msg[0] = '\0';
	switch (level){
		case LOG_DEBUG:
			strcat(log_msg, "DEBUG - ");
			break;
		case LOG_INFO:
			strcat(log_msg, "INFO - ");
			break;
		case LOG_WARN:
			strcat(log_msg, "WARN - ");
			break;
		case LOG_ERROR:
			strcat(log_msg, "ERROR - ");
			break;
		case LOG_FATAL:
			strcat(log_msg, "FATAL - ");
			break;
	}
	strcat(log_msg, msg);

	// ----- start thread lockdown -----
	pthread_mutex_lock(&log_msg_mutex);

	// ----- check the new string is fit into the message log -----
	size_t buffer_size;
	if (log_msg_start_pos == log_msg_end_pos) buffer_size = MSG_LOG_SIZE;
	else if (log_msg_start_pos < log_msg_end_pos) buffer_size = MSG_LOG_SIZE - log_msg_end_pos + log_msg_start_pos - 1;
	else if (log_msg_start_pos > log_msg_end_pos) buffer_size = log_msg_start_pos - log_msg_end_pos - 1;
	if (strlen(log_msg) > buffer_size) return -2; // when the message is larger than viable space

	// ----- move the string into the message log -----
	if (strlen(log_msg) + log_msg_end_pos < MSG_LOG_SIZE) { // case that it does not loop
		memcpy(log_msg_stogate + log_msg_end_pos + 1, log_msg, sizeof(log_msg));
	} else { // case that it loops around
		size_t first_chunk_size = MSG_LOG_SIZE - 1 - log_msg_end_pos;
		size_t second_chunk_size = strlen(log_msg) - first_chunk_size;
		memcpy(log_msg_stogate + log_msg_end_pos + 1, log_msg, first_chunk_size);
		memcpy(log_msg_stogate, log_msg + first_chunk_size, second_chunk_size);
	}

	// ----- update the end position -----
	log_msg_end_pos = (log_msg_end_pos + strlen(log_msg) + 1) % MSG_LOG_SIZE;

	// ----- end thread lockdown -----
	pthread_mutex_unlock(&log_msg_mutex);
	return 1;
}
