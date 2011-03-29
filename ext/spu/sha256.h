
# ifndef SHA256_H
# define SHA256_H

# include <stdint.h>
# include <spu_intrinsics.h>

typedef vec_uint4 hash_t[2];

extern const hash_t H0;

void sha256_round(hash_t, const uint32_t [16], const hash_t);

# endif
