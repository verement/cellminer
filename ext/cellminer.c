
# include <ruby.h>

extern void Init_spu_miner(VALUE);
extern void Init_ppu_miner(VALUE);

void Init_cellminer(void)
{
  VALUE mBitcoin;

  mBitcoin = rb_define_module("Bitcoin");

  Init_spu_miner(mBitcoin);
  Init_ppu_miner(mBitcoin);
}
