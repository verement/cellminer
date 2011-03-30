
# include <stdint.h>
# include <string.h>
# include <spu_intrinsics.h>

# include "worker.h"
# include "sha256.h"
# include "util.h"

/*
 * NAME:	verify_midstate()
 * DESCRIPTION:	check that the midstate received is correct
 */
static
int verify_midstate(const hash_t midstate, const uint32_t data[16])
{
  hash_t hash;

  sha256_update(&hash, data, H0);

  return memcmp(&hash, &midstate, sizeof(hash_t)) == 0;
}

/*
 * NAME:	work_on()
 * DESCRIPTION:	search for a solution and return 1 if found, 0 otherwise
 */
int work_on(struct worker_params *params)
{
  uint32_t *data = (uint32_t *) params->data;
  char buf[257];
  int64_t nonce;

  hex(buf, params->data, 128);
  debug("data     = %s", buf);

  debug_hash((const hash_t *) params->target,   "target  ");
  debug_hash((const hash_t *) params->midstate, "midstate");

  hex(buf, params->hash1, 64);
  debug("hash1    = %s", buf);

  if (!verify_midstate(*(const hash_t *) params->midstate, data)) {
    debug("midstate verification failed");
    return -1;
  }

  nonce = sha256_search(data, *(hash_t *) params->midstate,
			*(hash_t *) params->target);
  if (nonce < 0)
    return 0;

  /* store the found nonce */
  data[19] = nonce;

  return 1;
}
