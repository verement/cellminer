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

# ifndef UTIL_H
# define UTIL_H

# include <stdint.h>

# include "sha256.h"

# define   likely(expr)  __builtin_expect((expr), 1)
# define unlikely(expr)  __builtin_expect((expr), 0)

extern int debugging;

void hex(char *, const char *, uint32_t);

void debug(const char *, ...);
void debug_hash(const hash_t *, const char *);

# endif
