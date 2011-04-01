
# ifndef SHA256_H
# define SHA256_H

# include <stdint.h>
# include <spu_intrinsics.h>

typedef struct {
  uint32_t words[8] __attribute__ ((aligned (16)));
} hash_t;

extern const hash_t H0;

void sha256_update(hash_t *digest, const uint32_t M[16], const hash_t init);
int64_t sha256_search(uint32_t data[32],
		      const hash_t target, const hash_t midstate,
		      uint32_t start_nonce, uint32_t range);

# endif
