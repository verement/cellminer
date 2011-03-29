
# include <stdint.h>
# include <stddef.h>
# include <spu_mfcio.h>

# include "params.h"
# include "search.h"
# include "util.h"

/*
 * NAME:	dma_get()
 * DESCRIPTION:	initiate DMA transfer from main memory to LS
 */
static
void dma_get(volatile void *ls, uint64_t ea, uint32_t size, uint32_t tag)
{
  mfc_get(ls, ea, size, tag, 0, 0);
}

/*
 * NAME:	dma_put()
 * DESCRIPTION:	initiate DMA transfer from LS to main memory
 */
static
void dma_put(volatile void *ls, uint64_t ea, uint32_t size, uint32_t tag)
{
  mfc_put(ls, ea, size, tag, 0, 0);
}

/*
 * NAME:	dma_params()
 * DESCRIPTION:	transfer work parameters between LS and main memory
 */
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

/*
 * NAME:	main()
 * DESCRIPTION:	entry point for SPU program
 */
int main(uint64_t speid, uint64_t argp, uint64_t envp)
{
  spu_id = speid;

  /* loop forever loading parameters and doing work */

  while (1) {
    struct worker_params params __attribute__ ((aligned (128)));
    int result;

    debug("running (0x%llx)", argp);

    /* load parameters */

    if (__builtin_expect(dma_params(&params, argp, dma_get), 0))
      return WORKER_DMA_ERROR;

    debugging = params.flags & WORKER_FLAG_DEBUG;

    /* do work */

    if (__builtin_expect(result = work_on(&params), 0)) {
      if (result < 0)
	spu_stop(WORKER_VERIFY_ERROR);
      else {
	/* we found something? */
	if (dma_params(&params, argp, dma_put))
	  return WORKER_DMA_ERROR;

	spu_stop(WORKER_FOUND_SOMETHING);
      }
    }
    else
      spu_stop(WORKER_FOUND_NOTHING);
  }

  /* not reached */

  return 0;
}
