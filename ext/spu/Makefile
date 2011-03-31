
DEFINE		= -DDEBUG -DMAX_NONCE=0x8000000
OPTIMIZE	= -O3 -funroll-loops -fmodulo-sched -ftree-vectorize
WARN		= -Wall -Werror -Wstrict-aliasing=0

CC		= spu-gcc
AS		= spu-as
CFLAGS		= $(OPTIMIZE) $(WARN) $(INCLUDEPATH) $(DEFINE)
LDFLAGS		= $(LIBPATH)

worker.elf: main.o worker.o sha256.o util.o spu_slih.o spu_flih.o
	$(CC) -o $@ $(LDFLAGS) $^

test.elf: test.o sha256.o util.o
	$(CC) -o $@ $(LDFLAGS) $^

%.s: %.c
	$(CC) -S $(CFLAGS) -c $<

%-debug.s: %.c
	$(CC) -S -g $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	-rm -f *.elf *.o

depend.auto: *.[ch]
	@$(CC) -MM *.c >$@

*.o: Makefile
include depend.auto
