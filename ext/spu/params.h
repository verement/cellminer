
# ifndef SPU_PARAMS_H
# define SPU_PARAMS_H

struct worker_params {
  char target[32];
  char midstate[32];
  char hash1[64];
  char data[128];

  unsigned int flags;

  char padding[127];  /* required for proper DMA */
};

enum {
  WORKER_FOUND_NOTHING = 0,
  WORKER_FOUND_SOMETHING,
  WORKER_VERIFY_ERROR,
  WORKER_DMA_ERROR
};

enum {
  WORKER_FLAG_DEBUG = 0x0001
};

# endif
