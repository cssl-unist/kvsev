#ifndef MERKLE_CONFIG_H_
#define MERKLE_CONFIG_H_

#include <stdint.h>

#define HASH_LENGTH         32u
#define TREE_LEVELS         30u
#define MT_AL_MAX_ELEMS     536870912u

typedef uint8_t mt_hash_t[HASH_LENGTH];

typedef enum mt_error {
    MT_SUCCESS              = 0,
    MT_ERR_OUT_OF_MEMORY    = -1,
    MT_ERR_ILLEGAL_PARAM    = -2,
    MT_ERR_ILLEGAL_STATE    = -3,
    MT_ERR_ROOT_MISMATCH    = -4,
    MT_ERR_UNSPECIFIED      = -5
} mt_error_t;

#define MT_ERR_CHK(f) do {mt_error_t r = f; if (r != MT_SUCCESS) {return r;}} while(0)

#endif