#ifndef MERKLE_H_
#define MERKLE_H_

#include "merkle_config.h"
#include "mt_array_list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define MAX_LEAF    16
#define MAX_NODE    MAX_LEAF * 2 -1
#define ARITY       2

typedef struct merkle_tree {

	pthread_mutex_t 	mutex;
    uint32_t 			elems;
    mt_al_t 			*level[TREE_LEVELS];
} mt_t;

mt_t *mt_create(void);
void mt_delete(mt_t *mt);
mt_error_t mt_add(mt_t *mt, const uint8_t *tag, const size_t len);
mt_error_t mt_update(const mt_t *mt, const uint8_t *tag, const size_t len, const uint32_t offset);
mt_error_t mt_verify(const mt_t *mt, const uint8_t *tag, const size_t len, const uint32_t offset);
mt_error_t mt_truncate(mt_t *mt, uint32_t last_valid);
mt_error_t mt_get_root(mt_t *mt, mt_hash_t root);

uint32_t mt_get_size(const mt_t *mt);
int mt_exists(mt_t *mt, const uint32_t offset);
void mt_print_hash(const mt_hash_t hash);
void mt_print(const mt_t *mt);

#endif /* MERKLE_H_ */
