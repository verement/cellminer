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
  int64_t nonce;
  char buf[257];

  hex(buf, params->data.c, 128);
  debug("data     = %s", buf);

  debug_hash(&params->target.h,   "target  ");
  debug_hash(&params->midstate.h, "midstate");

  if (!verify_midstate(params->midstate.h,
		       params->data.m[0])) {
    debug("midstate verification failed");
    return -1;
  }

  debug("start_nonce = %08lx, range = %08lx",
	params->start_nonce, params->range);

  nonce = sha256_search(params->data.m[1],
			params->target.h,
			params->midstate.h,
			params->start_nonce,
			params->range);
  if (nonce < 0)
    return 0;

  /* store the found nonce */
  params->data.u[19] = nonce;

  return 1;
}
