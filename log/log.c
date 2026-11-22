#include "log.h"
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#define MSG_LOG_SIZE 512

static char log_msg_storage[MSG_LOG_SIZE];
static size_t log_msg_start_pos = 0;
static size_t log_msg_end_pos = 0;
static pthread_mutex_t log_msg_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t log_msg_read_cond = PTHREAD_COND_INITIALIZER;


/**
 * idea is to use circular array as log to hold message.
 */

void *log_init(void *arg){
	// ----- initialize the reading queue -----
	char msg_queue[MSG_LOG_SIZE];

	// ----- infinite loop to read log -----
	while (1) {

		// ----- obtain the lock -----
		pthread_mutex_lock(&log_msg_mutex);

		// ----- case the message log is empty -----
		if (log_msg_start_pos == log_msg_end_pos) pthread_cond_wait(&log_msg_read_cond, &log_msg_mutex); // wait until there is message in the log

		// ----- copy from the log into buffer -----
		size_t length;
		if (log_msg_start_pos <= log_msg_end_pos) length = log_msg_end_pos - log_msg_start_pos + 1;
		else length = MSG_LOG_SIZE - log_msg_start_pos + log_msg_end_pos + 1;
		size_t first = MSG_LOG_SIZE - log_msg_start_pos < length ? MSG_LOG_SIZE - log_msg_start_pos : length;
		memcpy(msg_queue, log_msg_storage + log_msg_start_pos, first);
		memcpy(msg_queue + first, log_msg_storage, length - first);

		// ----- update start position -----
		log_msg_start_pos = log_msg_end_pos;

		// ----- return the lock -----
		pthread_mutex_unlock(&log_msg_mutex);

		// ----- handle the log -----

	}

	// ----- return the lock -----
	pthread_mutex_unlock(&log_msg_mutex);
}

int log_write(log_level level, const char *msg, ...) {
	// ----- check params -----
	if (msg == NULL) return -1; // when no message passed

	// ----- create the string to be print out -----
	char log_msg[9 + strlen(msg) + 1]; // string lenght does not account for \0 at the tend
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
	size_t free_buffer_size;
	if (log_msg_start_pos == log_msg_end_pos) free_buffer_size = MSG_LOG_SIZE;
	else if (log_msg_start_pos < log_msg_end_pos) free_buffer_size = MSG_LOG_SIZE - log_msg_end_pos + log_msg_start_pos - 1;
	else if (log_msg_start_pos > log_msg_end_pos) free_buffer_size = log_msg_start_pos - log_msg_end_pos + 1;
	if (strlen(log_msg) + 1 > free_buffer_size) { // when the message is larger than viable space
		pthread_mutex_unlock(&log_msg_mutex);
		return -2;
	}

	// ----- move the string into the message log -----
	size_t first =  MSG_LOG_SIZE - log_msg_end_pos < strlen(log_msg) + 1 ? MSG_LOG_SIZE - log_msg_end_pos : sizeof(log_msg) + 1;
	memcpy(log_msg_storage + log_msg_end_pos + 1, log_msg, first);
	memcpy(log_msg_storage, log_msg + first, strlen(log_msg) - first + 1);

	// ----- update the end position -----
	log_msg_end_pos = (log_msg_end_pos + strlen(log_msg) + 1) % MSG_LOG_SIZE;

	// ----- end thread lockdown -----
	pthread_mutex_unlock(&log_msg_mutex);
	return 1;
}
