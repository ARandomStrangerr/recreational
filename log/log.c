#ifndef MSG_LOG_SIZE
#define MSG_LOG_SIZE 512
#endif

#include "log.h"
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

static char msg_log_buffer[MSG_LOG_SIZE];
static size_t log_msg_size = 0;
static pthread_mutex_t msg_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t msg_log_read_cond = PTHREAD_COND_INITIALIZER;


void *log_read(void *arg) {
	while (1) {
		// ----- temporary buffer to move msg into -----
		char temp_buffer[MSG_LOG_SIZE];

		// ----- start thread lockdown -----
		pthread_mutex_lock(&msg_log_mutex);

		// ----- check condition for lockup -----
		while (log_msg_size == 0) {
			pthread_cond_wait(&msg_log_read_cond, &msg_log_mutex);
		}

		// ----- copy into msg queue -----
		size_t length = log_msg_size;
		memcpy(temp_buffer, msg_log_buffer, length);
		log_msg_size = 0;

		// ----- release the thread lockdown -----
		pthread_mutex_unlock(&msg_log_mutex);

		// ----- start reading of string -----
		size_t offset = 0;
		while (offset < length) {
			const char *current_msg = temp_buffer + offset;
			size_t msg_len = strlen(temp_buffer);
			printf("%s\n", current_msg);
			offset += msg_len + 1;
		}
	}
}

int log_write(log_level level, const char *msg, ...) {
	// ----- check params -----
	if (msg == NULL) return -1; // when no message passed

	/*  ----- create the string to be print out -----
	 * the longest string is `debug - \0` with the length of 9.
	 * strcat(?, ?) find \0, override starting from that position, append \0 at the end.
	 * i.e msg = `ab\0`, log_level = 1; ex expect maximum length = 9 + 2 = 11
	 * initially, log_msg = '\0', length = 1 (accocunt for null terminator)
	 * after concat the prefix, log_msg = 'DEBUG - \0', length = 9 (account for null terminator)
	 * after concat the msg, log_msg = 'DEBUG - ab\0', length = 11 (account for null terminator)
	*/
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
	pthread_mutex_lock(&msg_log_mutex);

	/*  ----- check the new string is fit into the message log -----
	 * if the new string is shorter than available space, just reject to import it in
	 * couting the string has to count null terminator; therefore, the +1
	 * i.e 'abc\0', strlen(*) will count until \0 and exclude it; hence, it counts only 'abc'. +1 to count 'abc\0'
	 */
	size_t free_buffer_size = MSG_LOG_SIZE - log_msg_size;
	if (strlen(log_msg) + 1 > free_buffer_size) { // when the message is larger than viable space
		pthread_mutex_unlock(&msg_log_mutex);
		return -2;
	}

	// ----- write the string into buffer -----
	size_t bytes = strlen(log_msg) + 1;
	memcpy(msg_log_buffer + log_msg_size, log_msg, bytes);
	log_msg_size += bytes;

	// ----- notify that the buffer is ready to read -----
	pthread_cond_signal(&msg_log_read_cond);

	// ----- release the thread lockdown -----
	pthread_mutex_unlock(&msg_log_mutex);

	return 1;
}
