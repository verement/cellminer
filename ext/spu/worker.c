/*
 * cellminer - Bitcoin miner for the Cell Broadband Engine Architecture
 * Copyright © 2011 Robert Leslie
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (version 2) as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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
int verify_midstate(const hash_t midstate, const message_t data)
{
  hash_t hash;

  hash = sha256_update(data, H0);

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

  if (!verify_midstate(*(const hash_t *) params->midstate,
		       ((const message_t *) data)[0])) {
    debug("midstate verification failed");
    return -1;
  }

  debug("start_nonce = %08lx, range = %08lx",
	params->start_nonce, params->range);

  nonce = sha256_search(((const message_t *) params->data)[1],
			*(hash_t *) params->target,
			*(hash_t *) params->midstate,
			params->start_nonce,
			params->range);
  if (nonce < 0)
    return 0;

  /* store the found nonce */
  data[19] = nonce;

  return 1;
}
