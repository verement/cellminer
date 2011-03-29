
# ifndef UTIL_H
# define UTIL_H

# include "sha256.h"

extern uint64_t spu_id;
extern int debugging;

void hex(char *, const char *, uint32_t);

void debug(const char *, ...);
void debug_hash(const hash_t *, const char *);

# endif
