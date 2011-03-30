
DEFINE		= -DDEBUG
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

.PHONY: clean
clean:
	-rm -f *.elf *.o

depend.auto: *.[ch]
	@$(CC) -MM *.c >$@

include depend.auto
