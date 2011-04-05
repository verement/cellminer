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
