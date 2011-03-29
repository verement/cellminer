
# include <stdint.h>
# include <string.h>
# include <spu_intrinsics.h>

# include "search.h"
# include "sha256.h"
# include "util.h"

/*
 * NAME:	search()
 * DESCRIPTION:	search for suitable nonce to satisfy target
 */
static
int search(uint32_t *M, uint32_t hash1[16], hash_t midstate, hash_t target)
{
  uint32_t nonce;

  for (nonce = 0; nonce <= 0x1000; ++nonce) {
    hash_t hash;

    M[3] = nonce;  /* breaks strict-aliasing rules? */

    sha256_round((vec_uint4 *) hash1, M, midstate);
    sha256_round(hash, hash1, H0);

    /* spu_cntlz() */
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

  debug_hash((const hash_t *) &hash, "calculated midstate");

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

  debug("midstate verified");

  return search(&((uint32_t *) params->data)[16], (uint32_t *) params->hash1,
		(vec_uint4 *) params->midstate, (vec_uint4 *) params->target);
}
