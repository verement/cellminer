
DEFINE		= -DDEBUG
OPTIMIZE	= -O3 -funroll-loops -fmodulo-sched -ftree-vectorize
WARN		= -Wall -Werror #-Wstrict-aliasing=0

INCLUDEPATH	= -I$(SDK)/include
LIBPATH		= -L$(SDK)/lib

CC		= spu-gcc
AS		= spu-as
CFLAGS		= $(OPTIMIZE) $(WARN) $(INCLUDEPATH) $(DEFINE)
LDFLAGS		= $(LIBPATH)

SDK		= /opt/cell/sdk/prototype/usr/spu

worker.elf: worker.o sha256.o util.o search.o spu_slih.o spu_flih.o
	$(CC) -o $@ $(LDFLAGS) $^ -lspucrypto

test.elf: test.o sha256.o util.o
	$(CC) -o $@ $(LDFLAGS) $^

.PHONY: clean
clean:
	-rm -f *.elf *.o

depend.auto: *.[ch]
	@$(CC) -MM *.c >$@

include depend.auto
