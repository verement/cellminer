#
# cellminer - Bitcoin miner for the Cell Broadband Engine Architecture
# Copyright © 2011 Robert Leslie
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License (version 2) as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

DEFINE		 = -DUNROLL_SHA256
DEFINE		+= -Dinline="inline __attribute__((always_inline))"
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

%.s: %.c Makefile
	$(CC) -o $@ $(CFLAGS) -c $< -S

%-debug.s: %.c Makefile
	$(CC) -o $@ $(CFLAGS) -c $< -S -g

.PHONY: clean
clean:
	-rm -f *.elf *.o

depend.auto: *.[ch]
	@$(CC) -MM *.c >$@

*.o: Makefile
include depend.auto
