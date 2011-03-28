require 'mkmf'

$objs = ['spu_miner.o', 'spu_worker.o']
$CFLAGS << ' -Wall'

raise "missing libspe2" unless have_library('spe2', 'spe_context_run')

create_makefile 'spu_miner'
