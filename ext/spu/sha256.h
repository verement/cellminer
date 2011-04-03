
# ifndef SHA256_H
# define SHA256_H

# include <stdint.h>

typedef struct {
  uint32_t words[8] __attribute__ ((aligned (16)));
} hash_t;

typedef struct {
  uint32_t words[16] __attribute__ ((aligned (16)));
} message_t;

extern const hash_t H0;

hash_t sha256_update(const message_t M, const hash_t init);
int64_t sha256_search(const message_t data,
		      const hash_t target, const hash_t midstate,
		      uint32_t start_nonce, uint32_t range);

# endif
