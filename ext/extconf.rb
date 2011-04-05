require 'mkmf'

$objs = ['cellminer.o',
         'spu_miner.o', 'spu_worker.o',
         'ppu_miner.o', 'ppu/worker.a']

$CFLAGS << ' -Wall'

raise "missing libspe2" unless have_library('spe2', 'spe_context_run')

create_makefile 'cellminer'
