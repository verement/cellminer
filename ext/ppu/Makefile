
DEFINE		 = -DUNROLL_SHA256
#DEFINE		+= -DDEBUG
OPTIMIZE	 = -O3 -fno-strict-aliasing -funroll-loops
MACHINE		 = -mcpu=cell #-maltivec
DEBUG		 = -Wall -Werror

CFLAGS		 = $(DEFINE) $(OPTIMIZE) $(MACHINE) $(DEBUG) $(INCLUDEPATH)
LDFLAGS		 = $(LIBPATH)

CC		 = gcc

worker.a: worker.o sha256.o util.o
	$(AR) rc $@ $?

test: test.o sha256.o util.o
	$(CC) -o $@ $(LDFLAGS) $^

%.s: %.c Makefile
	$(CC) -o $@ $(CFLAGS) -c $< -S

%-debug.s: %.c Makefile
	$(CC) -o $@ $(CFLAGS) -c $< -S -g

.PHONY: clean
clean:
	-rm -f *.a *.o test

depend.auto: *.[ch]
	@$(CC) $(MACHINE) -MM *.c >$@

*.o: Makefile
include depend.auto
