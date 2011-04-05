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

# ifndef PARAMS_H
# define PARAMS_H

# include <stdint.h>

struct worker_params {
  char data[128];
  char target[32];
  char midstate[32];

  uint32_t start_nonce;
  uint32_t range;

  unsigned int flags;

  char padding[127];  /* required for proper DMA */
};

/* parameter flags */
enum {
  WORKER_FLAG_DEBUG = 0x0001
};

/* stop and signal values */
enum {
  WORKER_FOUND_NOTHING = 0,
  WORKER_FOUND_SOMETHING,

  WORKER_VERIFY_ERROR,
  WORKER_IRQ_SIGNAL
};

/* exit values */
enum {
  WORKER_DMA_ERROR = 1
};

# endif
