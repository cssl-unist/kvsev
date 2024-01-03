#include "merkle.h"
#include "mt_crypto.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef MT_DEBUG
#define DEBUG(m, ...) do {printf(m, __VA_ARGS__);} while(0);
#else
#define DEBUG(m, ...)
#endif

mt_t *mt_create(void) {
    mt_t *mt = calloc(1, sizeof(mt_t));
    if (!mt)
        return NULL;
    for (uint32_t i = 0; i < TREE_LEVELS; ++i) {
        mt_al_t *tmp = mt_al_create();
        if (!tmp) {
            for (uint32_t j = 0; j < i; ++j)
                mt_al_delete(mt->level[j]);
            free(mt);
            return NULL;

        }
        mt->level[i] = tmp;
    }
    return mt;
}

void mt_delete(mt_t *mt) {
    if (!mt) return;
    for (uint32_t i = 0; i < TREE_LEVELS; ++i)
        mt_al_delete(mt->level[i]);
    free(mt);
}

static int mt_right(uint32_t offset) {
    return offset & 0x01;
}

static int mt_left(uint32_t offset) {
    return !(offset & 0x01);
}

static void mt_init_hash(mt_hash_t hash, const uint8_t *tag, const size_t len) {
    assert(hash && tag && len <= HASH_LENGTH);
    memset(hash, 0, HASH_LENGTH);
    memcpy(hash, tag, len);
}

mt_error_t mt_add(mt_t *mt, const uint8_t *tag, const size_t len) {
    if (!(mt && tag && len <= HASH_LENGTH))
        return MT_ERR_ILLEGAL_PARAM;
    
    uint8_t message_digest[HASH_LENGTH];
    mt_init_hash(message_digest, tag, len);
    DEBUG("[MT_ADD][a][@%d] %s\n", mt->elems, mt_al_sprint_hex_buffer(message_digest, HASH_LENGTH))
    MT_ERR_CHK(mt_al_add(mt->level[0], message_digest));
    mt->elems += 1;
    if (mt->elems == 1)
        return MT_SUCCESS;

    uint32_t q = mt->elems - 1;
    uint32_t l = 0;
    while (q > 0 && l < TREE_LEVELS) {
        if (mt_right(q)) {
            uint8_t const * const left = mt_al_get(mt->level[l], q - 1);
            MT_ERR_CHK(mt_hash(left, message_digest, message_digest));
            MT_ERR_CHK(mt_al_add_or_update(mt->level[l + 1], message_digest, (q >> 1)));
        }
        q >>= 1;
        l += 1;
    }
    assert(!memcmp(message_digest, mt_al_get(mt->level[l], q), HASH_LENGTH));
    DEBUG("[MT_ADD][r][@%d] %s\n", 1, mt_al_sprint_hex_buffer(message_digest, HASH_LENGTH))
    return MT_SUCCESS;
}

uint32_t mt_get_size(const mt_t *mt) {
    if (!mt)
        return MT_ERR_ILLEGAL_PARAM;
    return mt_al_get_size(mt->level[0]);
}

int mt_exists(mt_t *mt, const uint32_t offset) {
    if (!mt || offset > MT_AL_MAX_ELEMS)
        return MT_ERR_ILLEGAL_PARAM;
    return (mt_al_get(mt->level[0], offset) != NULL);
}

static uint32_t hasNextLevelExceptRoot(mt_t const * const mt, uint32_t cur_lvl) {
    if (!mt) return 0;
    return (cur_lvl + 1 < TREE_LEVELS - 1) & (mt_al_get_size(mt->level[cur_lvl + 1]) > 0);
}

static const uint8_t *findRightNeighbor(const mt_t *mt, uint32_t offset, int32_t l) {
    if (!mt) return NULL;
    do {
        if (offset < mt_al_get_size(mt->level[l]))
            return mt_al_get(mt->level[l], offset);
        l -= 1;
        offset <<= 1;
    } while(l > -1);
    return NULL;
}

mt_error_t mt_verify(const mt_t *mt, const uint8_t *tag, const size_t len, const uint32_t offset) {
    if (!(mt && tag && len <= HASH_LENGTH && (offset < mt->elems)))
        return MT_ERR_ILLEGAL_PARAM;
    uint8_t message_digest[HASH_LENGTH];
    mt_init_hash(message_digest, tag, len);
    uint32_t q = offset;
    uint32_t l = 0;
    while (hasNextLevelExceptRoot(mt, l)) {
        if (!(q & 0x01)) {
            const uint8_t *right;
            if ((right = findRightNeighbor(mt, q + 1, l)) != NULL)
                MT_ERR_CHK(mt_hash(message_digest, right, message_digest));
        }
        else {
            uint8_t const * const left = mt_al_get(mt->level[l], q - 1);
            MT_ERR_CHK(mt_hash(left, message_digest, message_digest));
        }
        q >>= 1;
        l += 1;
    }
    int r = memcmp(message_digest, mt_al_get(mt->level[l], q), HASH_LENGTH);
    if (r)
        return MT_ERR_ROOT_MISMATCH;
    else
        return MT_SUCCESS;
}

mt_error_t mt_update(const mt_t *mt, const uint8_t *tag, const size_t len, const uint32_t offset) {
    if (!(mt && tag && len <= HASH_LENGTH && (offset < mt->elems)))
        return MT_ERR_ILLEGAL_PARAM;
    
    uint8_t message_digest[HASH_LENGTH];
    mt_init_hash(message_digest, tag, len);
    DEBUG("[MT_UPT][u][@%d] %s\n", offset, mt_al_sprint_hex_buffer(message_digest, HASH_LENGTH))
    MT_ERR_CHK(mt_al_update(mt->level[0], message_digest, offset));
    uint32_t q = offset;
    uint32_t l = 0;
    while (hasNextLevelExceptRoot(mt, l)) {
        if (mt_left(q)) {
            const uint8_t *right;
            if ((right = findRightNeighbor(mt, q + 1, l)) != NULL)
                MT_ERR_CHK(mt_hash(message_digest, right, message_digest));
        }
        else {
            uint8_t const * const left = mt_al_get(mt->level[l], q - 1);
            MT_ERR_CHK(mt_hash(left, message_digest, message_digest));
        }
        q >>= 1;
        l += 1;
        MT_ERR_CHK(mt_al_update_if_exists(mt->level[l], message_digest, q));
    }
    assert(!memcmp(message_digest, mt_al_get(mt->level[l], q), HASH_LENGTH));
    DEBUG("[MT_UPT][r][@%d] %s\n", l, mt_al_sprint_hex_buffer(message_digest, HASH_LENGTH))
    return MT_SUCCESS;
}

mt_error_t mt_get_root(mt_t *mt, mt_hash_t root) {
    if (!(mt && root))
        return MT_ERR_ILLEGAL_PARAM;
    uint32_t l = 0;
    while (hasNextLevelExceptRoot(mt, l))
        l += 1;
    memcpy(root, mt_al_get(mt->level[l], 0), sizeof(mt_hash_t));
    return MT_SUCCESS;
}

void mt_print_hash(const mt_hash_t hash) {
    if (!hash)
        printf("[ERROR][mt_print_hash]: Hash NULL");
    mt_al_print_hex_buffer(hash, HASH_LENGTH);
    printf("\n");
}

void mt_print(const mt_t *mt) {
    if (!mt) {
        fprintf(stderr, "[ERROR][mt_print]: Merkle tree NULL\n");
        return;
    }
    for (uint32_t i = 0; i < TREE_LEVELS; ++i) {
        if (mt->level[i]->elems == 0)
            return;
        fprintf(stderr, "==================== Merkle Tree level[%02u]: ====================\n", (unsigned int) i);
        mt_al_print(mt->level[i]);
    }
}