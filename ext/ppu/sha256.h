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

# ifndef SHA256_H
# define SHA256_H

# include <stdint.h>

typedef struct {
  uint32_t words[8] __attribute__ ((aligned (16)));
} hash_t;

typedef struct {
  uint32_t words[16] __attribute__ ((aligned (16)));
} message_t;

extern const hash_t H0;

hash_t sha256_update(const message_t M, const hash_t init);
int64_t sha256_search(const message_t data,
		      const hash_t target, const hash_t midstate,
		      uint32_t start_nonce, uint32_t range);

# endif
