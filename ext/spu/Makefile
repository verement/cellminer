
DEFINE		 = -DUNROLL_SHA256
#DEFINE		+= -DDEBUG
#OPTIMIZE	 = -O2 -fno-strict-aliasing -funroll-loops -ftree-vectorize
OPTIMIZE	 = -O3 -fno-strict-aliasing -funroll-loops
#OPTIMIZE	+= -fmodulo-sched -fmodulo-sched-allow-regmoves
MACHINE		 = -mno-safe-hints
WARN		 = -Wall -Werror
#WARN		+= -Wstrict-aliasing=0

CFLAGS		 = $(DEFINE) $(OPTIMIZE) $(MACHINE) $(WARN) $(INCLUDEPATH)
LDFLAGS		 = $(LIBPATH)

CC		 = spu-gcc
AS		 = spu-as

worker.elf: main.o worker.o sha256.o util.o spu_slih.o spu_flih.o
	$(CC) -o $@ $(LDFLAGS) $^

test.elf: test.o sha256.o util.o
	$(CC) -o $@ $(LDFLAGS) $^

%.s: %.c
	$(CC) -o $@ $(CFLAGS) -c $< -S

%-debug.s: %.c
	$(CC) -o $@ $(CFLAGS) -c $< -S -g

.PHONY: clean
clean:
	-rm -f *.elf *.o

depend.auto: *.[ch]
	@$(CC) -MM *.c >$@

*.o: Makefile
include depend.auto
