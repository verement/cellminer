
# ifndef SPU_PARAMS_H
# define SPU_PARAMS_H

struct worker_params {
  char data[128];
  char target[32];
  char midstate[32];
  char hash1[64];

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
