
# include <stdint.h>
# include <stddef.h>
# include <spu_intrinsics.h>
# include <spu_mfcio.h>

# ifdef DEBUG
#  include <stdarg.h>
#  include <stdio.h>
# endif

# include "params.h"

static uint64_t spu_id;

static
void debug(char *fmt, ...)
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

static
int work_on(struct worker_params *params)
{
  params->n += 100;
  return 1;

  return 0;
}

static
void dma_get(volatile void *ls, uint64_t ea, uint32_t size, uint32_t tag)
{
  mfc_get(ls, ea, size, tag, 0, 0);
}

static
void dma_put(volatile void *ls, uint64_t ea, uint32_t size, uint32_t tag)
{
  mfc_put(ls, ea, size, tag, 0, 0);
}

static
int dma_params(struct worker_params *params, uint64_t ea,
	       void (*dma)(volatile void *, uint64_t, uint32_t, uint32_t))
{
  uint32_t tag, size;

  tag = mfc_tag_reserve();
  if (tag == MFC_TAG_INVALID)
    return -1;

  size = (offsetof(struct worker_params, padding) + 127) & ~127;

  dma(params, ea, size, tag);

  mfc_write_tag_mask(1 << tag);
  mfc_read_tag_status_all();

  mfc_tag_release(tag);

  return 0;
}

int main(uint64_t speid, uint64_t argp, uint64_t envp)
{
  spu_id = speid;

  while (1) {
    struct worker_params params __attribute__ ((aligned (128)));

    debug("running (0x%llx)", argp);

    if (__builtin_expect(dma_params(&params, argp, dma_get), 0)) {
      debug("failed to load params");
      return WORKER_DMA_ERROR;
    }
    else {
      debug("loaded params (%d)", params.n);

      if (__builtin_expect(work_on(&params), 0)) {
	/* we found something? */
	if (dma_params(&params, argp, dma_put)) {
	  debug("failed to save params");
	  return WORKER_DMA_ERROR;
	}
	else {
	  debug("saved params (%d)", params.n);
	  spu_stop(WORKER_FOUND_SOMETHING);
	}
      }
      else
	spu_stop(WORKER_FOUND_NOTHING);
    }
  }

  return 0;
}
