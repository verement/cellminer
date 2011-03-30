
# include <stdint.h>
# include <string.h>
# include <spu_intrinsics.h>

# include <sha.h>

# include "search.h"
# include "sha256.h"
# include "util.h"

/*
 * NAME:	search()
 * DESCRIPTION:	search for suitable nonce to satisfy target
 */
static
int search(uint32_t data[32], uint32_t hash1[16], hash_t target)
{
  SHA256_CTX midstate, hash1_init;
  uint64_t nonce;

  SHA256_Init(&midstate);
  SHA256_Update_fast(&midstate, (vec_uchar16 *) data, 64);

  SHA256_Init(&hash1_init);

  for (nonce = 0; nonce < 0x10000000ULL; ++nonce) {
    SHA256_CTX ctx = midstate, hash1 = hash1_init;
    hash_t hash;
    char hexdigest[65];
    vec_uchar16 reverse = {
      15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
    };
    vec_uchar16 shufborrow = {
      4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0x80, 0x80, 0x80, 0xc0
    };
    vec_uint4 rhash[2], borrow;

    data[19] = nonce;

    SHA256_Update(&ctx, &data[16], 16);
    SHA256_Final((void *) hash, &ctx);

    SHA256_Update(&hash1, hash, 32);
    SHA256_Final((void *) hash, &hash1);

    rhash[0] = spu_shuffle(hash[1], hash[1], reverse);

    borrow = spu_genb(target[0], rhash[0]);

    if (__builtin_expect(spu_extract(borrow, 0) == 0, 1))
      continue;

    rhash[1] = spu_shuffle(hash[0], hash[0], reverse);

    hex(hexdigest, (char *) rhash, 32);
    debug("nonce %08x = %s", nonce, hexdigest);

    /* perform subtraction */
    borrow = spu_shuffle(borrow, borrow, shufborrow);

    hex(hexdigest, (char *) &borrow, 16);
    debug("shuffle borrow = %s", hexdigest);

    borrow = spu_genbx(target[0], rhash[0], borrow);

    hex(hexdigest, (char *) &borrow, 16);
    debug("         genbx = %s", hexdigest);

    if (__builtin_expect(spu_extract(borrow, 0) == 0, 1))
      continue;

    debug("found solution!");
    return 1;
  }

  return 0;
}

/*
 * NAME:	verify_midstate()
 * DESCRIPTION:	check that the midstate received is correct
 */
static
int verify_midstate(const hash_t *midstate, const uint32_t data[16])
{
  hash_t hash;

  sha256_round(hash, data, H0);

  return memcmp(hash, midstate, sizeof(hash_t)) == 0;
}

/*
 * NAME:	work_on()
 * DESCRIPTION:	search for a solution and return 1 if found, 0 otherwise
 */
int work_on(struct worker_params *params)
{
  char buf[257];

  hex(buf, params->data, 128);
  debug("data     = %s", buf);

  debug_hash((const hash_t *) params->target,   "target  ");
  debug_hash((const hash_t *) params->midstate, "midstate");

  hex(buf, params->hash1, 64);
  debug("hash1    = %s", buf);

  if (!verify_midstate((const hash_t *) params->midstate,
		       (uint32_t *) params->data)) {
    debug("midstate verification failed");
    return -1;
  }

  return search((uint32_t *) params->data, (uint32_t *) params->hash1,
		(vec_uint4 *) params->target);
}
