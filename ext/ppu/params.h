
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
};

/* parameter flags */
enum {
  WORKER_FLAG_DEBUG = 0x0001
};

/* return values */
enum {
  WORKER_FOUND_NOTHING = 0,
  WORKER_FOUND_SOMETHING,

  WORKER_VERIFY_ERROR
};

# endif
