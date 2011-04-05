
# include <stdint.h>
# include <string.h>

# include "params.h"
# include "sha256.h"
# include "util.h"

/*
 * NAME:	verify_midstate()
 * DESCRIPTION:	check that the midstate received is correct
 */
static
int verify_midstate(const hash_t midstate, const message_t data)
{
  hash_t hash;

  hash = sha256_update(data, H0);

  return memcmp(&hash, &midstate, sizeof(hash_t)) == 0;
}

/*
 * NAME:	PPU->mine()
 * DESCRIPTION:	search for a solution and return 1 if found, 0 otherwise
 */
int ppu_mine(struct worker_params *params)
{
  uint32_t *data = (uint32_t *) params->data;
  char buf[257];
  int64_t nonce;

  debugging = params->flags & WORKER_FLAG_DEBUG;

  hex(buf, params->data, 128);
  debug("data     = %s", buf);

  debug_hash((const hash_t *) params->target,   "target  ");
  debug_hash((const hash_t *) params->midstate, "midstate");

  if (!verify_midstate(*(const hash_t *) params->midstate,
		       ((const message_t *) data)[0])) {
    debug("midstate verification failed");
    return WORKER_VERIFY_ERROR;
  }

  debug("start_nonce = %08lx, range = %08lx",
	params->start_nonce, params->range);

  nonce = sha256_search(((const message_t *) params->data)[1],
			*(hash_t *) params->target,
			*(hash_t *) params->midstate,
			params->start_nonce,
			params->range);
  if (nonce < 0)
    return WORKER_FOUND_NOTHING;

  /* store the found nonce */
  data[19] = nonce;

  return WORKER_FOUND_SOMETHING;
}
