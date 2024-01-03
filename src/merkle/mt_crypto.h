#ifndef MT_CRYPTO_H_
#define MT_CRYPTO_H_

#include "merkle_config.h"

mt_error_t mt_hash(const mt_hash_t left, const mt_hash_t right, mt_hash_t message_digest);

#endif /* MT_CRYPTO_H_ */