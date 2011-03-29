
# ifdef DEBUG
#  include <stdarg.h>
#  include <stdio.h>
# endif

# include "util.h"
# include "sha256.h"

uint64_t spu_id;

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

  hex(buf, (const char *) hash, 32);
  debug("%s = %s", desc, buf);
# endif
}
