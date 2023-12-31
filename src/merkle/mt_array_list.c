#include "mt_array_list.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static uint32_t round_next_power_two(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    v += (v == 0);
    return v;
}

static int is_power_of_two(uint32_t v) {
    return (v != 0) && ((v & (v - 1)) == 0);
}

mt_al_t *mt_al_create(void) {
    return calloc(1, sizeof(mt_al_t));
}

void mt_al_delete(mt_al_t *mt_al) {
    free(mt_al->store);
    free(mt_al);
}

mt_error_t mt_al_add(mt_al_t *mt_al, const mt_hash_t hash) {
    assert(mt_al->elems < MT_AL_MAX_ELEMS);
    if (!(mt_al && hash))
        return MT_ERR_ILLEGAL_PARAM;
    if (mt_al->elems == 0) {
        mt_al->store = malloc(HASH_LENGTH);
        if (!mt_al->store)
            return MT_ERR_OUT_OF_MEMORY;
    }
    else if (is_power_of_two(mt_al->elems)) {
        if (((mt_al->elems << 1) < mt_al->elems)
            || (mt_al->elems << 1 > MT_AL_MAX_ELEMS))
            return MT_ERR_ILLEGAL_STATE;
        
        size_t alloc = mt_al->elems * 2 * HASH_LENGTH;
        uint8_t *tmp = realloc(mt_al->store, alloc);
        if (!tmp)
            return MT_ERR_OUT_OF_MEMORY;
        
        mt_al->store = tmp;
    }

    memcpy(&mt_al->store[mt_al->elems * HASH_LENGTH], hash, HASH_LENGTH);
    mt_al->elems += 1;
    return MT_SUCCESS;
}

mt_error_t mt_al_update(const mt_al_t *mt_al, const mt_hash_t hash, const uint32_t offset) {
    assert(mt_al->elems < MT_AL_MAX_ELEMS);
    if (!(mt_al && hash && offset < mt_al->elems))
        return MT_ERR_ILLEGAL_PARAM;
    memcpy(&mt_al->store[offset * HASH_LENGTH], hash, HASH_LENGTH);
    return MT_SUCCESS;
}

mt_error_t mt_al_update_if_exists(const mt_al_t *mt_al, const mt_hash_t hash, const uint32_t offset) {
    assert(mt_al->elems < MT_AL_MAX_ELEMS);
    if (!(mt_al && hash))
        return MT_ERR_ILLEGAL_PARAM;
    if (offset >= mt_al->elems)
        return MT_SUCCESS;
    memcpy(&mt_al->store[offset * HASH_LENGTH], hash, HASH_LENGTH);
    return MT_SUCCESS;
}

mt_error_t mt_al_add_or_update(mt_al_t *mt_al, const mt_hash_t hash, const uint32_t offset) {
    assert(mt_al->elems < MT_AL_MAX_ELEMS);
    if (!(mt_al && hash) || offset > mt_al->elems)
        return MT_ERR_ILLEGAL_PARAM;
    if (offset == mt_al->elems)
        return mt_al_add(mt_al, hash);
    else
        return mt_al_update(mt_al, hash, offset);
}

mt_error_t mt_al_truncate(mt_al_t *mt_al, const uint32_t elems) {
    assert(mt_al->elems < MT_AL_MAX_ELEMS);
    if (!(mt_al && elems < mt_al->elems))
        return MT_ERR_ILLEGAL_PARAM;
    mt_al->elems = elems;
    if (elems == 0) {
        free(mt_al->store);
        return MT_SUCCESS;
    }
    uint32_t alloc = round_next_power_two(elems) * HASH_LENGTH;
    uint8_t *tmp = realloc(mt_al->store, alloc);
    if (!tmp)
        return MT_ERR_OUT_OF_MEMORY;
    mt_al->store = tmp;
    return MT_SUCCESS;
}

const uint8_t *mt_al_get(const mt_al_t *mt_al, const uint32_t offset) {
    assert(mt_al->elems < MT_AL_MAX_ELEMS);
    if (!(mt_al && offset < mt_al->elems))
        return NULL;
    return &mt_al->store[offset * HASH_LENGTH];
}

void mt_al_print_hex_buffer(const uint8_t *buffer, const size_t size) {
    if (!buffer) {
        fprintf(stderr, "[ERROR][mt_al_print_hex_buffer]: Merkle tree array list is NULL");
        return;
    }
    for (size_t i = 0; i < size; ++i)
        fprintf(stderr, "%02X", buffer[i]);
}

char *mt_al_sprint_hex_buffer(const uint8_t *buffer, const size_t size) {
    if (!buffer) {
        fprintf(stderr, "[ERROR][mt_al_sprint_hex_buffer]: Merkle tree array list in NULL");
        return NULL;
    }
    size_t to_alloc = size * (sizeof(char) * 2) + 1;
    char *str = malloc(to_alloc);
    for (size_t i = 0; i < size; ++i)
        snprintf((str + (i * 2)), 3, "%02X", buffer[i]);
    return str;
}

void mt_al_print(const mt_al_t *mt_al) {
    assert(mt_al->elems < MT_AL_MAX_ELEMS);
    if (!mt_al) {
        fprintf(stderr, "[ERROR][mt_al_print]: Merkle tree array list is NULL");
        return;
    }
    fprintf(stderr, "[%08X\n", (unsigned int)mt_al->elems);
    for (uint32_t i = 0; i < mt_al->elems; ++i) {
        mt_al_print_hex_buffer(&mt_al->store[i * HASH_LENGTH], HASH_LENGTH);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "]\n");
}