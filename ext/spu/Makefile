
CC		= spu-gcc
CFLAGS		= -Wall $(OPTIMIZE) -DDEBUG
OPTIMIZE	= -O3 -funroll-loops -fmodulo-sched -ftree-vectorize

SDK		= /opt/cell/sdk/prototype/usr/spu

spu_worker.elf: worker.o
	$(CC) $(LDFLAGS) -o $@ $<

worker.o: worker.c params.h

.PHONY: clean
clean:
	-rm -f *.elf *.o
