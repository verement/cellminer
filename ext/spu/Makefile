
CC	= spu-gcc

spu_worker.elf: worker.o
	$(CC) -o $@ $<

worker.o: worker.c

.PHONY: clean
clean:
	-rm -f *.elf *.o
