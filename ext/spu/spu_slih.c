
# include <spu_intrinsics.h>

# include "spu_slih.h"

# define SPU_EVENT_ID(_mask) (spu_extract(spu_cntlz(spu_promote(_mask, 0)), 0))

static
unsigned int default_slih(unsigned int events)
{
  unsigned int mse;

  /* get my (left-most) event number */
  mse = 0x80000000 >> SPU_EVENT_ID(events);

  /* return updated event bits */
  return events & ~mse;
}

spu_slih_func spu_slih_handlers[33] __attribute__ ((aligned (16))) = {
  default_slih,
  default_slih,
  default_slih,
  default_slih,
  default_slih,
  default_slih,
  default_slih,
  default_slih,

  default_slih,
  default_slih,
  default_slih,
  default_slih,
  default_slih,
  default_slih,
  default_slih,
  default_slih,

  default_slih,
  default_slih,
  default_slih,
  default_slih,  /* event bit 19: Multisource Sync */
  default_slih,  /* event bit 20: Privilege attention */
  default_slih,  /* event bit 21: Lock-line reservation-lost */
  default_slih,  /* event bit 22: Signal-notification 1 */
  default_slih,  /* event bit 23: Signal-notification 2 */

  default_slih,  /* event bit 24: SPU Write Outbound Mailbox available */
  default_slih,  /* event bit 25: SPU Write Outbound IRQ Mailbox avail. */
  default_slih,  /* event bit 26: Decrementer */
  default_slih,  /* event bit 27: SPE mailbox */
  default_slih,  /* event bit 28: DMA-queue */
  default_slih,  /* reserved */
  default_slih,  /* extra bit 30: DMA list command stall-and-notify */
  default_slih,  /* extra bit 31: SPE tag-status update */

  default_slih   /* extra slih table entry for phantom events */
};

void spu_slih_register(unsigned int mask, spu_slih_func func)
{
  unsigned int id;

  while (mask) {
    id = SPU_EVENT_ID(mask);		/* get next (left-most) event */
    spu_slih_handlers[id] = func;	/* put function ptr into slih table */
    mask &= ~(0x80000000 >> id);	/* clear that event bit */
  }
}
