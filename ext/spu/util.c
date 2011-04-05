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

# ifdef DEBUG
#  include <stdarg.h>
#  include <stdio.h>
# endif

# include "util.h"
# include "sha256.h"

uint64_t spu_id;
int debugging;

# define HEXDIGITS "0123456789abcdef"

void hex(char *buf, const char *ptr, uint32_t len)
{
  while (len--) {
    *buf++ = HEXDIGITS[*ptr   / 16];
    *buf++ = HEXDIGITS[*ptr++ % 16];
  }

  *buf = 0;
}

void debug(const char *fmt, ...)
{
# ifdef DEBUG
  va_list args;

  if (!debugging)
    return;

  va_start(args, fmt);
  fprintf(stderr, "SPU(%08llx): ", spu_id);
  vfprintf(stderr, fmt, args);
  fputc('\n', stderr);
  va_end(args);
# endif
}

void debug_hash(const hash_t *hash, const char *desc)
{
# ifdef DEBUG
  char buf[65];

  if (!debugging)
    return;

  hex(buf, (const char *) hash, 32);
  debug("%s = %s", desc, buf);
# endif
}
