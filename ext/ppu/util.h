
# ifndef UTIL_H
# define UTIL_H

# include <stdint.h>

# include "sha256.h"

extern int debugging;

void hex(char *, const char *, uint32_t);

void debug(const char *, ...);
void debug_hash(const hash_t *, const char *);

# endif
