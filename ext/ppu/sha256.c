
# include <stdint.h>
# include <stdlib.h>
# include <altivec.h>
# include <vec_types.h>

# include "sha256.h"

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
  return vec_sr(x, vec_splat_u32(n));
}

static inline
uint32_t ROTR(unsigned int n, uint32_t x)
{
  return (x >> n) | (x << (32 - n));
}

static inline
vec_uint4 vec_ROTR(unsigned int n, vec_uint4 x)
{
  return vec_rl(x, vec_splats(32 - n));
}

static inline
uint32_t Ch(uint32_t x, uint32_t y, uint32_t z)
{
  return (x & y) ^ (~x & z);
}

static inline
vec_uint4 vec_Ch(vec_uint4 x, vec_uint4 y, vec_uint4 z)
{
  return vec_sel(z, y, x);
}

static inline
uint32_t Maj(uint32_t x, uint32_t y, uint32_t z)
{
  return (x & y) ^ (x & z) ^ (y & z);
}

static inline
vec_uint4 vec_Maj(vec_uint4 x, vec_uint4 y, vec_uint4 z)
{
  return vec_sel(vec_and(y, z), vec_or(y, z), x);
}

static inline
uint32_t Sigma0(uint32_t x)
{
  return ROTR(2, x) ^ ROTR(13, x) ^ ROTR(22, x);
}

static inline
vec_uint4 vec_Sigma0(vec_uint4 x)
{
  return vec_xor(vec_xor(vec_ROTR( 2, x),
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
  return vec_xor(vec_xor(vec_ROTR( 6, x),
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
  return vec_xor(vec_xor(vec_ROTR( 7, x),
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
  return vec_xor(vec_xor(vec_ROTR(17, x),
			 vec_ROTR(19, x)),
		 vec_SHR(10, x));
}

# define ADD(a, b)  (a + b)
# define SPLAT(x)   (x)

# define W(t)					\
  ADD(sigma1(W[(t - 2) % 16]),			\
      ADD(W[(t - 7) % 16],			\
	  ADD(sigma0(W[(t - 15) % 16]),		\
	      W[t % 16])))

# define T1(t, e, f, g, h)			\
  ADD(h,					\
      ADD(Sigma1(e),				\
	  ADD(Ch(e, f, g),			\
	      ADD(W[t % 16],			\
		  SPLAT(K[t])))))

# define T2(a, b, c)				\
  ADD(Sigma0(a), Maj(a, b, c))

# define ROUND(t)				\
  do {						\
    T1 = T1(t, e, f, g, h);			\
    T2 = T2(a, b, c);				\
    h = g;					\
    g = f;					\
    f = e;					\
    e = ADD(d, T1);				\
    d = c;					\
    c = b;					\
    b = a;					\
    a = ADD(T1, T2);				\
  } while (0)

/*
 * NAME:	sha256->update()
 * DESCRIPTION:	update SHA-256 digest with a single message block
 */
hash_t sha256_update(const message_t M, const hash_t init)
{
  hash_t digest;
# if !defined(UNROLL_SHA256)
  int t;
# endif
  uint32_t W[16], a, b, c, d, e, f, g, h, T1, T2;

  a = init.words[0];
  b = init.words[1];
  c = init.words[2];
  d = init.words[3];
  e = init.words[4];
  f = init.words[5];
  g = init.words[6];
  h = init.words[7];

# ifdef UNROLL_SHA256
  W[ 0] = M.words[ 0]; ROUND( 0);
  W[ 1] = M.words[ 1]; ROUND( 1);
  W[ 2] = M.words[ 2]; ROUND( 2);
  W[ 3] = M.words[ 3]; ROUND( 3);
  W[ 4] = M.words[ 4]; ROUND( 4);
  W[ 5] = M.words[ 5]; ROUND( 5);
  W[ 6] = M.words[ 6]; ROUND( 6);
  W[ 7] = M.words[ 7]; ROUND( 7);

  W[ 8] = M.words[ 8]; ROUND( 8);
  W[ 9] = M.words[ 9]; ROUND( 9);
  W[10] = M.words[10]; ROUND(10);
  W[11] = M.words[11]; ROUND(11);
  W[12] = M.words[12]; ROUND(12);
  W[13] = M.words[13]; ROUND(13);
  W[14] = M.words[14]; ROUND(14);
  W[15] = M.words[15]; ROUND(15);
# else
  for (t = 0; t < 16; ++t) {
    W[t] = M.words[t];
    ROUND(t);
  }
# endif

# ifdef UNROLL_SHA256
  W[16 % 16] = W(16); ROUND(16);
  W[17 % 16] = W(17); ROUND(17);
  W[18 % 16] = W(18); ROUND(18);
  W[19 % 16] = W(19); ROUND(19);
  W[20 % 16] = W(20); ROUND(20);
  W[21 % 16] = W(21); ROUND(21);
  W[22 % 16] = W(22); ROUND(22);
  W[23 % 16] = W(23); ROUND(23);

  W[24 % 16] = W(24); ROUND(24);
  W[25 % 16] = W(25); ROUND(25);
  W[26 % 16] = W(26); ROUND(26);
  W[27 % 16] = W(27); ROUND(27);
  W[28 % 16] = W(28); ROUND(28);
  W[29 % 16] = W(29); ROUND(29);
  W[30 % 16] = W(30); ROUND(30);
  W[31 % 16] = W(31); ROUND(31);

  W[32 % 16] = W(32); ROUND(32);
  W[33 % 16] = W(33); ROUND(33);
  W[34 % 16] = W(34); ROUND(34);
  W[35 % 16] = W(35); ROUND(35);
  W[36 % 16] = W(36); ROUND(36);
  W[37 % 16] = W(37); ROUND(37);
  W[38 % 16] = W(38); ROUND(38);
  W[39 % 16] = W(39); ROUND(39);

  W[40 % 16] = W(40); ROUND(40);
  W[41 % 16] = W(41); ROUND(41);
  W[42 % 16] = W(42); ROUND(42);
  W[43 % 16] = W(43); ROUND(43);
  W[44 % 16] = W(44); ROUND(44);
  W[45 % 16] = W(45); ROUND(45);
  W[46 % 16] = W(46); ROUND(46);
  W[47 % 16] = W(47); ROUND(47);

  W[48 % 16] = W(48); ROUND(48);
  W[49 % 16] = W(49); ROUND(49);
  W[50 % 16] = W(50); ROUND(50);
  W[51 % 16] = W(51); ROUND(51);
  W[52 % 16] = W(52); ROUND(52);
  W[53 % 16] = W(53); ROUND(53);
  W[54 % 16] = W(54); ROUND(54);
  W[55 % 16] = W(55); ROUND(55);

  W[56 % 16] = W(56); ROUND(56);
  W[57 % 16] = W(57); ROUND(57);
  W[58 % 16] = W(58); ROUND(58);
  W[59 % 16] = W(59); ROUND(59);
  W[60 % 16] = W(60); ROUND(60);
  W[61 % 16] = W(61); ROUND(61);
  W[62 % 16] = W(62); ROUND(62);
  W[63 % 16] = W(63); ROUND(63);
# else
  for (t = 16; t < 64; ++t) {
    W[t % 16] = W(t);
    ROUND(t);
  }
# endif

  digest.words[0] = ADD(a, init.words[0]);
  digest.words[1] = ADD(b, init.words[1]);
  digest.words[2] = ADD(c, init.words[2]);
  digest.words[3] = ADD(d, init.words[3]);
  digest.words[4] = ADD(e, init.words[4]);
  digest.words[5] = ADD(f, init.words[5]);
  digest.words[6] = ADD(g, init.words[6]);
  digest.words[7] = ADD(h, init.words[7]);

  return digest;
}

# define Ch	vec_Ch
# define Maj	vec_Maj
# define Sigma0	vec_Sigma0
# define Sigma1	vec_Sigma1
# define sigma0	vec_sigma0
# define sigma1	vec_sigma1

# undef  ADD
# define ADD(a, b)  vec_add(a, b)

# undef  SPLAT
# define SPLAT(x)  vec_splats(x)

# define GENB(a, b)  vec_subc(a, b)

static inline
vec_uint4 GENBX(vec_uint4 a, vec_uint4 b, vec_uint4 c)
{
  return vec_and(vec_or(vec_cmpgt(a, b),
			vec_and(vec_cmpeq(a, b), c)),
		 vec_splat_u32(1));
}

/*
 * NAME:	sha256->search()
 * DESCRIPTION:	try to find a nonce which satisfies a target hash
 */
int64_t sha256_search(const message_t M,
		      const hash_t target, const hash_t midstate,
		      uint32_t start_nonce, uint32_t range)
{
  uint32_t nonce, stop_nonce = start_nonce + range + (4 - (range % 4)) % 4;
# if !defined(UNROLL_SHA256)
  int t;
# endif
  vec_uint4 W0[3], a0, b0, c0, d0, e0, f0, g0, h0;
  vec_uint4 W[16], a, b, c, d, e, f, g, h, T1, T2;
  vec_uint4 borrow;
  const vec_uchar16 reverse_endian = {
     3,  2,  1,  0,
     7,  6,  5,  4,
    11, 10,  9,  8,
    15, 14, 13, 12
  };

  /* precompute first three rounds */

  a = SPLAT(midstate.words[0]);
  b = SPLAT(midstate.words[1]);
  c = SPLAT(midstate.words[2]);
  d = SPLAT(midstate.words[3]);
  e = SPLAT(midstate.words[4]);
  f = SPLAT(midstate.words[5]);
  g = SPLAT(midstate.words[6]);
  h = SPLAT(midstate.words[7]);

# ifdef UNROLL_SHA256
  W[0] = SPLAT(M.words[0]); ROUND(0);
  W[1] = SPLAT(M.words[1]); ROUND(1);
  W[2] = SPLAT(M.words[2]); ROUND(2);
# else
  for (t = 0; t < 3; ++t) {
    W[t] = SPLAT(M.words[t]);
    ROUND(t);
  }
# endif

  W0[0] = W[0];
  W0[1] = W[1];
  W0[2] = W[2];

  a0 = a;
  b0 = b;
  c0 = c;
  d0 = d;
  e0 = e;
  f0 = f;
  g0 = g;
  h0 = h;

  /* do the search, four at a time */

  for (nonce = start_nonce; nonce != stop_nonce; nonce += 4) {
    W[0] = W0[0];
    W[1] = W0[1];
    W[2] = W0[2];

    a = a0;
    b = b0;
    c = c0;
    d = d0;
    e = e0;
    f = f0;
    g = g0;
    h = h0;

    /* t = 3 */
    W[3] = (vec_uint4) { nonce + 0, nonce + 1, nonce + 2, nonce + 3 };
    ROUND(3);

# ifdef UNROLL_SHA256
    W[ 4] = SPLAT(M.words[ 4]); ROUND( 4);
    W[ 5] = SPLAT(M.words[ 5]); ROUND( 5);
    W[ 6] = SPLAT(M.words[ 6]); ROUND( 6);
    W[ 7] = SPLAT(M.words[ 7]); ROUND( 7);

    W[ 8] = SPLAT(M.words[ 8]); ROUND( 8);
    W[ 9] = SPLAT(M.words[ 9]); ROUND( 9);
    W[10] = SPLAT(M.words[10]); ROUND(10);
    W[11] = SPLAT(M.words[11]); ROUND(11);
    W[12] = SPLAT(M.words[12]); ROUND(12);
    W[13] = SPLAT(M.words[13]); ROUND(13);
    W[14] = SPLAT(M.words[14]); ROUND(14);
    W[15] = SPLAT(M.words[15]); ROUND(15);
# else
    for (t = 4; t < 16; ++t) {
      W[t] = SPLAT(M.words[t]);
      ROUND(t);
    }
# endif

# ifdef UNROLL_SHA256
    W[16 % 16] = W(16); ROUND(16);
    W[17 % 16] = W(17); ROUND(17);
    W[18 % 16] = W(18); ROUND(18);
    W[19 % 16] = W(19); ROUND(19);
    W[20 % 16] = W(20); ROUND(20);
    W[21 % 16] = W(21); ROUND(21);
    W[22 % 16] = W(22); ROUND(22);
    W[23 % 16] = W(23); ROUND(23);

    W[24 % 16] = W(24); ROUND(24);
    W[25 % 16] = W(25); ROUND(25);
    W[26 % 16] = W(26); ROUND(26);
    W[27 % 16] = W(27); ROUND(27);
    W[28 % 16] = W(28); ROUND(28);
    W[29 % 16] = W(29); ROUND(29);
    W[30 % 16] = W(30); ROUND(30);
    W[31 % 16] = W(31); ROUND(31);

    W[32 % 16] = W(32); ROUND(32);
    W[33 % 16] = W(33); ROUND(33);
    W[34 % 16] = W(34); ROUND(34);
    W[35 % 16] = W(35); ROUND(35);
    W[36 % 16] = W(36); ROUND(36);
    W[37 % 16] = W(37); ROUND(37);
    W[38 % 16] = W(38); ROUND(38);
    W[39 % 16] = W(39); ROUND(39);

    W[40 % 16] = W(40); ROUND(40);
    W[41 % 16] = W(41); ROUND(41);
    W[42 % 16] = W(42); ROUND(42);
    W[43 % 16] = W(43); ROUND(43);
    W[44 % 16] = W(44); ROUND(44);
    W[45 % 16] = W(45); ROUND(45);
    W[46 % 16] = W(46); ROUND(46);
    W[47 % 16] = W(47); ROUND(47);

    W[48 % 16] = W(48); ROUND(48);
    W[49 % 16] = W(49); ROUND(49);
    W[50 % 16] = W(50); ROUND(50);
    W[51 % 16] = W(51); ROUND(51);
    W[52 % 16] = W(52); ROUND(52);
    W[53 % 16] = W(53); ROUND(53);
    W[54 % 16] = W(54); ROUND(54);
    W[55 % 16] = W(55); ROUND(55);

    W[56 % 16] = W(56); ROUND(56);
    W[57 % 16] = W(57); ROUND(57);
    W[58 % 16] = W(58); ROUND(58);
    W[59 % 16] = W(59); ROUND(59);
    W[60 % 16] = W(60); ROUND(60);
    W[61 % 16] = W(61); ROUND(61);
    W[62 % 16] = W(62); ROUND(62);
    W[63 % 16] = W(63); ROUND(63);
# else
    for (t = 16; t < 64; ++t) {
      W[t % 16] = W(t);
      ROUND(t);
    }
# endif

    W[0] = ADD(a, SPLAT(midstate.words[0]));
    W[1] = ADD(b, SPLAT(midstate.words[1]));
    W[2] = ADD(c, SPLAT(midstate.words[2]));
    W[3] = ADD(d, SPLAT(midstate.words[3]));
    W[4] = ADD(e, SPLAT(midstate.words[4]));
    W[5] = ADD(f, SPLAT(midstate.words[5]));
    W[6] = ADD(g, SPLAT(midstate.words[6]));
    W[7] = ADD(h, SPLAT(midstate.words[7]));

    /* first SHA-256 complete */

    a = SPLAT(H0.words[0]);
    b = SPLAT(H0.words[1]);
    c = SPLAT(H0.words[2]);
    d = SPLAT(H0.words[3]);
    e = SPLAT(H0.words[4]);
    f = SPLAT(H0.words[5]);
    g = SPLAT(H0.words[6]);
    h = SPLAT(H0.words[7]);

    ROUND(0);
    ROUND(1);
    ROUND(2);
    ROUND(3);
    ROUND(4);
    ROUND(5);
    ROUND(6);
    ROUND(7);

    W[ 8] = SPLAT(0x80000000U); ROUND( 8);

# ifdef UNROLL_SHA256
    W[ 9] = SPLAT(0x00000000U); ROUND( 9);
    W[10] = SPLAT(0x00000000U); ROUND(10);
    W[11] = SPLAT(0x00000000U); ROUND(11);
    W[12] = SPLAT(0x00000000U); ROUND(12);
    W[13] = SPLAT(0x00000000U); ROUND(13);
    W[14] = SPLAT(0x00000000U); ROUND(14);
# else
    for (t = 9; t < 15; ++t) {
      W[t] = SPLAT(0U);
      ROUND(t);
    }
# endif

    W[15] = SPLAT(0x00000100U); ROUND(15);

# ifdef UNROLL_SHA256
    W[16 % 16] = W(16); ROUND(16);
    W[17 % 16] = W(17); ROUND(17);
    W[18 % 16] = W(18); ROUND(18);
    W[19 % 16] = W(19); ROUND(19);
    W[20 % 16] = W(20); ROUND(20);
    W[21 % 16] = W(21); ROUND(21);
    W[22 % 16] = W(22); ROUND(22);
    W[23 % 16] = W(23); ROUND(23);

    W[24 % 16] = W(24); ROUND(24);
    W[25 % 16] = W(25); ROUND(25);
    W[26 % 16] = W(26); ROUND(26);
    W[27 % 16] = W(27); ROUND(27);
    W[28 % 16] = W(28); ROUND(28);
    W[29 % 16] = W(29); ROUND(29);
    W[30 % 16] = W(30); ROUND(30);
    W[31 % 16] = W(31); ROUND(31);

    W[32 % 16] = W(32); ROUND(32);
    W[33 % 16] = W(33); ROUND(33);
    W[34 % 16] = W(34); ROUND(34);
    W[35 % 16] = W(35); ROUND(35);
    W[36 % 16] = W(36); ROUND(36);
    W[37 % 16] = W(37); ROUND(37);
    W[38 % 16] = W(38); ROUND(38);
    W[39 % 16] = W(39); ROUND(39);

    W[40 % 16] = W(40); ROUND(40);
    W[41 % 16] = W(41); ROUND(41);
    W[42 % 16] = W(42); ROUND(42);
    W[43 % 16] = W(43); ROUND(43);
    W[44 % 16] = W(44); ROUND(44);
    W[45 % 16] = W(45); ROUND(45);
    W[46 % 16] = W(46); ROUND(46);
    W[47 % 16] = W(47); ROUND(47);

    W[48 % 16] = W(48); ROUND(48);
    W[49 % 16] = W(49); ROUND(49);
    W[50 % 16] = W(50); ROUND(50);
    W[51 % 16] = W(51); ROUND(51);
    W[52 % 16] = W(52); ROUND(52);
    W[53 % 16] = W(53); ROUND(53);
    W[54 % 16] = W(54); ROUND(54);
    W[55 % 16] = W(55); ROUND(55);

    W[56 % 16] = W(56); ROUND(56);
    W[57 % 16] = W(57); ROUND(57);
    W[58 % 16] = W(58); ROUND(58);
    W[59 % 16] = W(59); ROUND(59);
    /* t = 60..63 delayed */
# else
    for (t = 16; t < 60; ++t) {
      W[t % 16] = W(t);
      ROUND(t);
    }
# endif

    W[60 % 16] = W(60);
    T1 = T1(60, e, f, g, h);

    T2 = ADD(ADD(d, T1), SPLAT(H0.words[7]));

    /* quick check to see if any element of the last word vector is zero */
    if (__builtin_expect(vec_any_eq(T2, vec_splat_u32(0)) == 0, 1))
      continue;

    /* we have something interesting; finish the SHA-256 */

    ROUND(60);

# ifdef UNROLL_SHA256
    W[61 % 16] = W(61); ROUND(61);
    W[62 % 16] = W(62); ROUND(62);
    W[63 % 16] = W(63); ROUND(63);
# else
    for (t = 61; t < 64; ++t) {
      W[t % 16] = W(t);
      ROUND(t);
    }
# endif

    a = ADD(a, SPLAT(H0.words[0]));
    b = ADD(b, SPLAT(H0.words[1]));
    c = ADD(c, SPLAT(H0.words[2]));
    d = ADD(d, SPLAT(H0.words[3]));
    e = ADD(e, SPLAT(H0.words[4]));
    f = ADD(f, SPLAT(H0.words[5]));
    g = ADD(g, SPLAT(H0.words[6]));
    h = ADD(h, SPLAT(H0.words[7]));

    /* now do the full (reversed-endian) subtraction */

    borrow = GENB(SPLAT(target.words[7]),
		  vec_perm(a, a, reverse_endian));
    borrow = GENBX(SPLAT(target.words[6]),
		   vec_perm(b, b, reverse_endian), borrow);
    borrow = GENBX(SPLAT(target.words[5]),
		   vec_perm(c, c, reverse_endian), borrow);
    borrow = GENBX(SPLAT(target.words[4]),
		   vec_perm(d, d, reverse_endian), borrow);
    borrow = GENBX(SPLAT(target.words[3]),
		   vec_perm(e, e, reverse_endian), borrow);
    borrow = GENBX(SPLAT(target.words[2]),
		   vec_perm(f, f, reverse_endian), borrow);
    borrow = GENBX(SPLAT(target.words[1]),
		   vec_perm(g, g, reverse_endian), borrow);
    borrow = GENBX(SPLAT(target.words[0]),
		   vec_perm(h, h, reverse_endian), borrow);

    if (__builtin_expect(vec_all_eq(borrow, vec_splat_u32(0)), 1))
      continue;

    /* we have a winner */

    if (vec_extract(borrow, 0) == 1)
      return nonce + 0;
    if (vec_extract(borrow, 1) == 1)
      return nonce + 1;
    if (vec_extract(borrow, 2) == 1)
      return nonce + 2;
    if (vec_extract(borrow, 3) == 1)
      return nonce + 3;

    abort();
  }

  return -1;
}
