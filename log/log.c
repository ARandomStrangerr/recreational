#include "log.h"
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#define MSG_LOG_SIZE 512

static char msg_log_buffer[MSG_LOG_SIZE];
static size_t log_msg_start_pos = 0;
static size_t log_msg_size = 0;
static pthread_mutex_t msg_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t msg_log_read_cond = PTHREAD_COND_INITIALIZER;


/**
 * idea is to use circular array as log to hold message.
 */


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

	/* ----- calculate starting position to cpy -----
	 * using formula writing position = (start + length) % size
	 * there are 2 case:
	 * case #1: string not wrap around yet
	 * let the string is `abc`; start = 0, length = 3, size = 10
	 * (0+3)%10 = 3
	 * we can start to write at position 3
	 * visualization:
	 * [0][1][2][3][4][5][6][7][8][9]
	 * [a][b][c][ ][ ][ ][ ][ ][ ][ ]
	 * case #2: sting wrap around
	 * let the string is `abc`; start = 8, length = 3, size = 10
	 * (8+3)%10 = 1
	 * we can start to write at position 1
	 * visualization:
	 * [0][1][2][3][4][5][6][7][8][9]
	 * [c][ ][ ][ ][ ][ ][ ][ ][a][b]
	 * of course, we still need +1 to account fo `\0`
	 */
	size_t start_write_pos = (log_msg_start_pos + log_msg_size) % MSG_LOG_SIZE;

	/* ----- cpy the temporary msg into the actual msg log -----
	 * because we ensure that the message fit into buffer
	 * therefore the situation where it write over start position is impossible
	 * we only have to accounted for wrapping around when writing.
	 * case #1: not wrap around
	 * let the string be `def`; start writing position = 2, length = 3, size = 10
	 * just copy the string in, it is that simple
	 * [0][1][2][3][4][5][6][7][8][9]
	 * [b][c][d][e][f][ ][ ][ ][ ][a]
	 * case #2: wrap around
	 * let the string be `def`; start writing position = 8, length = 3, size = 10
	 * copy in 2 char at the end and 1 char at the beginning
	 * [0][1][2][3][4][5][6][7][8][9]
	 * [f][ ][ ][ ][ ][a][b][c][d][e]
	 * we can achieve both by do 2 copy:
	 * cpy #1: the size of the message itself or the size till the end, whichever is smaller
	 * cpy #2: the remain of the message.
	 * if we cpy the message itself, then there is nothing left to cpy
	 * if we cpy till the end of, then secon cpy handle the remain of the msg
	 */
	size_t first_chunk = strlen(log_msg) + 1 < MSG_LOG_SIZE - start_write_pos ? strlen(log_msg) + 1 : MSG_LOG_SIZE - start_write_pos;
	memcpy(msg_log_buffer + start_write_pos, log_msg, first_chunk);
	memcpy(msg_log_buffer, log_msg + first_chunk, strlen(log_msg) + 1 - first_chunk);

	// ----- update the end position -----
	log_msg_size += strlen(log_msg) + 1;

	// ----- end thread lockdown -----
	pthread_mutex_unlock(&msg_log_mutex);
	return 1;
}
