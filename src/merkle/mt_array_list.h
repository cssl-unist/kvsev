#ifndef MT_ARRAY_LIST_H_
#define MT_ARRAY_LIST_H_

#include "merkle_config.h"
#include <stdlib.h>

typedef struct merkle_tree_array_list {
    uint32_t elems;
    uint8_t *store;
} mt_al_t;

mt_al_t *mt_al_create(void);
void mt_al_delete(mt_al_t *mt_al);
mt_error_t mt_al_add(mt_al_t *mt_al, const mt_hash_t hash);
mt_error_t mt_al_update(const mt_al_t *mt_al, const mt_hash_t hash, const uint32_t offset);
mt_error_t mt_al_update_if_exists(const mt_al_t *mt_al, const mt_hash_t hash, const uint32_t offset);
mt_error_t mt_al_add_or_update(mt_al_t *mt_al, const mt_hash_t hash, const uint32_t offset);
mt_error_t mt_al_truncate(mt_al_t *mt_al, const uint32_t elems);

const uint8_t *mt_al_get(const mt_al_t *mt_al, const uint32_t offset);

static inline uint32_t mt_al_has_right_neighbor(const mt_al_t *mt_al, const uint32_t offset) {
    if (!mt_al)
        return 0;
    return (offset + 1) < mt_al->elems;
}

static inline uint32_t mt_al_get_size(const mt_al_t *mt_al) {
    if (!mt_al)
        return 0;
    return mt_al->elems;
}

/* Debug-print functions */
void mt_al_print_hex_buffer(const uint8_t *buffer, const size_t size);
char *mt_al_sprint_hex_buffer(const uint8_t *buffer, const size_t size);
void mt_al_print(const mt_al_t *mt_al);

#endif