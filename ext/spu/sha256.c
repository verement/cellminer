
# include <stdint.h>
# include <spu_intrinsics.h>

# include "sha256.h"

const hash_t H0 = {
  { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a },
  { 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 }
};

static
const uint32_t K[64] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
  0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
  0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,

  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
  0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
  0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,

  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
  0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
  0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,

  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
  0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
  0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static inline
uint32_t SHR(unsigned int n, uint32_t x)
{
  return x >> n;
}

static inline
vec_uint4 vSHR(unsigned int n, vec_uint4 x)
{
  return spu_sr(x, n);
}

static inline
uint32_t ROTR(unsigned int n, uint32_t x)
{
  return (x >> n) | (x << (32 - n));
}

static inline
vec_uint4 vROTR(unsigned int n, vec_uint4 x)
{
  return spu_rl(x, 32 - n);
}

static inline
uint32_t Ch(uint32_t x, uint32_t y, uint32_t z)
{
  return (x & y) ^ (~x & z);
}

static inline
vec_uint4 vCh(vec_uint4 x, vec_uint4 y, vec_uint4 z)
{
  return spu_xor(spu_and(x, y),
		 spu_andc(z, x));
}

static inline
uint32_t Maj(uint32_t x, uint32_t y, uint32_t z)
{
  return (x & y) ^ (x & z) ^ (y & z);
}

static inline
vec_uint4 vMaj(vec_uint4 x, vec_uint4 y, vec_uint4 z)
{
  return spu_xor(spu_xor(spu_and(x, y),
			 spu_and(x, z)),
		 spu_and(y, z));
}

static inline
uint32_t SIGMA0(uint32_t x)
{
  return ROTR(2, x) ^ ROTR(13, x) ^ ROTR(22, x);
}

static inline
vec_uint4 vSIGMA0(vec_uint4 x)
{
  return spu_xor(spu_xor(vROTR( 2, x),
			 vROTR(13, x)),
		 vROTR(22, x));
}

static inline
uint32_t SIGMA1(uint32_t x)
{
  return ROTR(6, x) ^ ROTR(11, x) ^ ROTR(25, x);
}

static inline
vec_uint4 vSIGMA1(vec_uint4 x)
{
  return spu_xor(spu_xor(vROTR( 6, x),
			 vROTR(11, x)),
		 vROTR(25, x));
}

static inline
uint32_t sigma0(uint32_t x)
{
  return ROTR(7, x) ^ ROTR(18, x) ^ SHR(3, x);
}

static inline
vec_uint4 vsigma0(vec_uint4 x)
{
  return spu_xor(spu_xor(vROTR( 7, x),
			 vROTR(18, x)),
		 vSHR(3, x));
}

static inline
uint32_t sigma1(uint32_t x)
{
  return ROTR(17, x) ^ ROTR(19, x) ^ SHR(10, x);
}

static inline
vec_uint4 vsigma1(vec_uint4 x)
{
  return spu_xor(spu_xor(vROTR(17, x),
			 vROTR(19, x)),
		 vSHR(10, x));
}

/*
 * NAME:	sha256->round()
 * DESCRIPTION:	perform one round of SHA-256 on a single message block
 */
void sha256_round(hash_t digest, const uint32_t M[16], const hash_t init)
{
  int t;
  uint32_t W[16], T1, T2;
  vec_uint4 abcd, efgh;

  abcd = init[0];
  efgh = init[1];

# define a (((uint32_t *) &abcd)[0])
# define b (((uint32_t *) &abcd)[1])
# define c (((uint32_t *) &abcd)[2])
# define d (((uint32_t *) &abcd)[3])

# define e (((uint32_t *) &efgh)[0])
# define f (((uint32_t *) &efgh)[1])
# define g (((uint32_t *) &efgh)[2])
# define h (((uint32_t *) &efgh)[3])

  for (t = 0; t < 16; ++t) {
    W[t] = M[t];

    T1 = h + SIGMA1(e) + Ch(e, f, g) + K[t] + W[t];
    T2 = SIGMA0(a) + Maj(a, b, c);

    efgh = spu_srqwbyte(efgh, 4);
    e = d + T1;
    abcd = spu_srqwbyte(abcd, 4);
    a = T1 + T2;
  }

  for (t = 16; t < 64; ++t) {
    W[t % 16] = sigma1(W[(t - 2) % 16]) + W[(t - 7) % 16] +
      sigma0(W[(t - 15) % 16]) + W[(t - 16) % 16];

    T1 = h + SIGMA1(e) + Ch(e, f, g) + K[t] + W[t % 16];
    T2 = SIGMA0(a) + Maj(a, b, c);

    efgh = spu_srqwbyte(efgh, 4);
    e = d + T1;
    abcd = spu_srqwbyte(abcd, 4);
    a = T1 + T2;
  }

# undef a
# undef b
# undef c
# undef d

# undef e
# undef f
# undef g
# undef h

  digest[0] = spu_add(abcd, init[0]);
  digest[1] = spu_add(efgh, init[1]);
}
