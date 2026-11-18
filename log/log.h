#ifndef LOG
#define LOG

typedef enum {
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL
} log_level;

/**
 * @brief ~ consumer function. a thread consuming
 */
void *log_init(void *);

/**
 * @brief ~ producer function. threads will call and pass message into this thread
 *
 * @param level ~ type of the message
 * @param msg ~ message for the log
 *
 * @return
 * -1 if the message is NULL
 * -2 if the message is bigger than viable space
 * 1 if success
 */
int log_write(log_level, const char *, ...);

void log_shutdown();

#endif
