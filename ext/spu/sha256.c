
# include <stdint.h>
# include <spu_intrinsics.h>

# include "sha256.h"
# include "util.h"

const hash_t H0 = {
  { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 }
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
vec_uint4 vec_SHR(unsigned int n, vec_uint4 x)
{
  return spu_sr(x, n);
}

static inline
uint32_t ROTR(unsigned int n, uint32_t x)
{
  return (x >> n) | (x << (32 - n));
}

static inline
vec_uint4 vec_ROTR(unsigned int n, vec_uint4 x)
{
  return spu_rl(x, 32 - n);
}

static inline
uint32_t Ch(uint32_t x, uint32_t y, uint32_t z)
{
  return (x & y) ^ (~x & z);
}

static inline
vec_uint4 vec_Ch(vec_uint4 x, vec_uint4 y, vec_uint4 z)
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
vec_uint4 vec_Maj(vec_uint4 x, vec_uint4 y, vec_uint4 z)
{
  return spu_xor(spu_xor(spu_and(x, y),
			 spu_and(x, z)),
		 spu_and(y, z));
}

static inline
uint32_t Sigma0(uint32_t x)
{
  return ROTR(2, x) ^ ROTR(13, x) ^ ROTR(22, x);
}

static inline
vec_uint4 vec_Sigma0(vec_uint4 x)
{
  return spu_xor(spu_xor(vec_ROTR( 2, x),
			 vec_ROTR(13, x)),
		 vec_ROTR(22, x));
}

static inline
uint32_t Sigma1(uint32_t x)
{
  return ROTR(6, x) ^ ROTR(11, x) ^ ROTR(25, x);
}

static inline
vec_uint4 vec_Sigma1(vec_uint4 x)
{
  return spu_xor(spu_xor(vec_ROTR( 6, x),
			 vec_ROTR(11, x)),
		 vec_ROTR(25, x));
}

static inline
uint32_t sigma0(uint32_t x)
{
  return ROTR(7, x) ^ ROTR(18, x) ^ SHR(3, x);
}

static inline
vec_uint4 vec_sigma0(vec_uint4 x)
{
  return spu_xor(spu_xor(vec_ROTR( 7, x),
			 vec_ROTR(18, x)),
		 vec_SHR(3, x));
}

static inline
uint32_t sigma1(uint32_t x)
{
  return ROTR(17, x) ^ ROTR(19, x) ^ SHR(10, x);
}

static inline
vec_uint4 vec_sigma1(vec_uint4 x)
{
  return spu_xor(spu_xor(vec_ROTR(17, x),
			 vec_ROTR(19, x)),
		 vec_SHR(10, x));
}

# define ADD(a, b)  (a + b)

# define W(t)							\
  ADD(sigma1(W[(t - 2) % 16]),					\
      ADD(W[(t - 7) % 16],					\
	  ADD(sigma0(W[(t - 15) % 16]),				\
	      W[(t - 16) % 16])))
# define T1(t, e, f, g, h)					\
  ADD(h,							\
      ADD(Sigma1(e),						\
	  ADD(Ch(e, f, g),					\
	      ADD(W[t % 16],					\
		  K[t]))))
# define T2(a, b, c)						\
  ADD(Sigma0(a), Maj(a, b, c))

# define ROUNDX(t, na, nb, nc, nd, ne, nf, ng, nh, a, b, c, d, e, f, g, h) \
  do {									\
    T1 = T1(t, e, f, g, h);						\
    T2 = T2(a, b, c);							\
    nh = g;								\
    ng = f;								\
    nf = e;								\
    ne = ADD(d, T1);							\
    nd = c;								\
    nc = b;								\
    nb = a;								\
    na = ADD(T1, T2);							\
  } while (0)

# define ROUND(t, a, b, c, d, e, f, g, h)			\
  ROUNDX(t, a, b, c, d, e, f, g, h,				\
	    a, b, c, d, e, f, g, h)

/*
 * NAME:	sha256->update()
 * DESCRIPTION:	update SHA-256 digest with a single message block
 */
void sha256_update(hash_t *digest, const uint32_t M[16], const hash_t init)
{
  int t;
  uint32_t W[16], a, b, c, d, e, f, g, h, T1, T2;

  a = init.words[0];
  b = init.words[1];
  c = init.words[2];
  d = init.words[3];
  e = init.words[4];
  f = init.words[5];
  g = init.words[6];
  h = init.words[7];

  for (t = 0; t < 16; ++t) {
    W[t] = M[t];
    ROUND(t, a, b, c, d, e, f, g, h);
  }

  for (t = 16; t < 64; ++t) {
    W[t % 16] = W(t);
    ROUND(t, a, b, c, d, e, f, g, h);
  }

  digest->words[0] = ADD(a, init.words[0]);
  digest->words[1] = ADD(b, init.words[1]);
  digest->words[2] = ADD(c, init.words[2]);
  digest->words[3] = ADD(d, init.words[3]);
  digest->words[4] = ADD(e, init.words[4]);
  digest->words[5] = ADD(f, init.words[5]);
  digest->words[6] = ADD(g, init.words[6]);
  digest->words[7] = ADD(h, init.words[7]);
}

# define Ch	vec_Ch
# define Maj	vec_Maj
# define Sigma0	vec_Sigma0
# define Sigma1	vec_Sigma1
# define sigma0	vec_sigma0
# define sigma1	vec_sigma1

# undef  ADD
# define ADD(a, b)  spu_add(a, b)

# define VHASHWORD(hash, i)  spu_splats((hash).words[i])

/*
 * NAME:	sha256->search()
 * DESCRIPTION:	try to find a nonce which satisfies a target hash
 */
int64_t sha256_search(uint32_t data[32], const hash_t midstate,
		      const hash_t target)
{
  uint64_t nonce;
  int t;
  uint32_t *M = &data[16];
  vec_uint4 W0[3], a0, b0, c0, d0, e0, f0, g0, h0;
  vec_uint4 W[16], a, b, c, d, e, f, g, h, T1, T2;
  vec_uint4 a2, b2, c2, d2, e2, f2, g2, h2;
  vec_uint4 ta, tb, tc, td, te, tf, tg, th;
  vec_uint4 borrow;
  vec_uchar16 reverse_endian = {
     3,  2,  1,  0,
     7,  6,  5,  4,
    11, 10,  9,  8,
    15, 14, 13, 12
  };
  unsigned int solution;

  /* set up target vectors */

  ta = VHASHWORD(target, 0);
  tb = VHASHWORD(target, 1);
  tc = VHASHWORD(target, 2);
  td = VHASHWORD(target, 3);
  te = VHASHWORD(target, 4);
  tf = VHASHWORD(target, 5);
  tg = VHASHWORD(target, 6);
  th = VHASHWORD(target, 7);

  /* precompute first three rounds */

  a0 = VHASHWORD(midstate, 0);
  b0 = VHASHWORD(midstate, 1);
  c0 = VHASHWORD(midstate, 2);
  d0 = VHASHWORD(midstate, 3);
  e0 = VHASHWORD(midstate, 4);
  f0 = VHASHWORD(midstate, 5);
  g0 = VHASHWORD(midstate, 6);
  h0 = VHASHWORD(midstate, 7);

  for (t = 0; t < 3; ++t) {
    W[t] = W0[t] = spu_splats(M[t]);
    ROUND(t, a0, b0, c0, d0, e0, f0, g0, h0);
  }

  /* do the search, four at a time */

  for (nonce = 0; nonce <= 0x100000000ULL; nonce += 4) {
    W[0] = W0[0];
    W[1] = W0[1];
    W[2] = W0[2];

    /* t = 3 */
    W[3] = (vec_uint4) { nonce + 0, nonce + 1, nonce + 2, nonce + 3 };
    ROUNDX(3, a, b, c, d, e, f, g, h, a0, b0, c0, d0, e0, f0, g0, h0);

    for (t = 4; t < 16; ++t) {
      W[t] = spu_splats(M[t]);
      ROUND(t, a, b, c, d, e, f, g, h);
    }

    for (t = 16; t < 64; ++t) {
      W[t % 16] = W(t);
      ROUND(t, a, b, c, d, e, f, g, h);
    }

    a = ADD(a, VHASHWORD(midstate, 0));
    b = ADD(b, VHASHWORD(midstate, 1));
    c = ADD(c, VHASHWORD(midstate, 2));
    d = ADD(d, VHASHWORD(midstate, 3));
    e = ADD(e, VHASHWORD(midstate, 4));
    f = ADD(f, VHASHWORD(midstate, 5));
    g = ADD(g, VHASHWORD(midstate, 6));
    h = ADD(h, VHASHWORD(midstate, 7));

    /* first SHA-256 complete */

    W[0] = a;
    ROUNDX(0, a2, b2, c2, d2, e2, f2, g2, h2,
	   VHASHWORD(H0, 0), VHASHWORD(H0, 1),
	   VHASHWORD(H0, 2), VHASHWORD(H0, 3),
	   VHASHWORD(H0, 4), VHASHWORD(H0, 5),
	   VHASHWORD(H0, 6), VHASHWORD(H0, 7));
    W[1] = b;
    ROUND(1, a2, b2, c2, d2, e2, f2, g2, h2);
    W[2] = c;
    ROUND(2, a2, b2, c2, d2, e2, f2, g2, h2);
    W[3] = d;
    ROUND(3, a2, b2, c2, d2, e2, f2, g2, h2);
    W[4] = e;
    ROUND(4, a2, b2, c2, d2, e2, f2, g2, h2);
    W[5] = f;
    ROUND(5, a2, b2, c2, d2, e2, f2, g2, h2);
    W[6] = g;
    ROUND(6, a2, b2, c2, d2, e2, f2, g2, h2);
    W[7] = h;
    ROUND(7, a2, b2, c2, d2, e2, f2, g2, h2);

    W[8] = spu_splats(0x80000000U);
    ROUND(8, a2, b2, c2, d2, e2, f2, g2, h2);

    for (t = 9; t < 15; ++t) {
      W[t] = spu_splats(0U);
      ROUND(t, a2, b2, c2, d2, e2, f2, g2, h2);
    }

    W[15] = spu_splats(0x100U);
    ROUND(15, a2, b2, c2, d2, e2, f2, g2, h2);

    for (t = 16; t < 64; ++t) {
      W[t % 16] = W(t);
      ROUND(t, a2, b2, c2, d2, e2, f2, g2, h2);
    }

    a2 = ADD(a2, VHASHWORD(H0, 0));
    b2 = ADD(b2, VHASHWORD(H0, 1));
    c2 = ADD(c2, VHASHWORD(H0, 2));
    d2 = ADD(d2, VHASHWORD(H0, 3));
    e2 = ADD(e2, VHASHWORD(H0, 4));
    f2 = ADD(f2, VHASHWORD(H0, 5));
    g2 = ADD(g2, VHASHWORD(H0, 6));
    h2 = ADD(h2, VHASHWORD(H0, 7));

    /* second SHA-256 complete */

    /* reverse the endian of the last word vector */
    a = spu_shuffle(h2, h2, reverse_endian);

    /* generate borrow (target - a) and check */
    borrow = spu_genb(ta, a);

    if (__builtin_expect(spu_extract(spu_gather(borrow), 0) == 0, 1))
      continue;

    /* we may have something interesting; do full subtraction */

    debug("interesting nonce %08llx+3", nonce);

    h = spu_shuffle(a2, a2, reverse_endian);
    borrow = spu_genb(th, h);
    g = spu_shuffle(b2, b2, reverse_endian);
    borrow = spu_genbx(tg, g, borrow);
    f = spu_shuffle(c2, c2, reverse_endian);
    borrow = spu_genbx(tf, f, borrow);
    e = spu_shuffle(d2, d2, reverse_endian);
    borrow = spu_genbx(te, e, borrow);
    d = spu_shuffle(e2, e2, reverse_endian);
    borrow = spu_genbx(td, d, borrow);
    c = spu_shuffle(f2, f2, reverse_endian);
    borrow = spu_genbx(tc, c, borrow);
    b = spu_shuffle(g2, g2, reverse_endian);
    borrow = spu_genbx(tb, b, borrow);

    borrow = spu_genbx(ta, a, borrow);

    solution = spu_extract(spu_gather(borrow), 0);

    if (__builtin_expect(solution == 0, 1))
      continue;

    /* we have a winner */

    return (uint32_t) nonce +
      spu_extract(spu_cntlz(spu_promote(solution, 0)), 0) - 28;
  }

  return -1;
}
