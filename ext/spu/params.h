
enum {
  WORKER_FOUND_NOTHING = 0,
  WORKER_FOUND_SOMETHING,
  WORKER_DMA_ERROR,
};

struct worker_params {
  int n;

  char padding[127];  /* required for proper DMA */
};
