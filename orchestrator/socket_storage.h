#ifndef SOC_STORAGE
#define SOC_STORAGE

#include "../communication/socket.h"


typedef struct {
	soc_client_socket* soc;
	char *name;
	char *params;
} soc_storage_entry;

/**
 * @brief allocate the array in heap
 */
int soc_storage_create_storage(soc_storage_entry *[]);

/**
 * @brief append an entry
 */
int soc_storage_append(const soc_client_socket *, const char *, const char *);

/**
 * @brief remove an entry based on index
 */
int soc_storage_remove(const int idx);

/**
 * @brief get an entry based on index
 */
int soc_storage_get_by_idx(soc_storage_entry *);

/**
 * @brief get an entry based on name
 */
int sock_storage_get_by_name(const char *);
#endif
