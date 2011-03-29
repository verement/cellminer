
CC		= spu-gcc
CFLAGS		= -Wall -Werror $(OPTIMIZE) -DDEBUG
OPTIMIZE	= -O3 -funroll-loops -fmodulo-sched -ftree-vectorize

worker.elf: worker.o sha256.o util.o
	$(CC) $(LDFLAGS) -o $@ $^

test.elf: test.o sha256.o util.o
	$(CC) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	-rm -f *.elf *.o

depend: *.[ch]
	@$(CC) -MM *.c >$@

include depend
