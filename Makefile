
RUBY	= ruby1.9.1

ext/cellminer.so: ext/Makefile ext/*.[ch]  \
		ext/spu/Makefile ext/spu/*.[chs]  \
		ext/ppu/Makefile ext/ppu/*.[chs]
	$(MAKE) -C ext

ext/Makefile: ext/extconf.rb ext/depend
	cd ext && $(RUBY) extconf.rb

.PHONY: clean
clean:
	$(MAKE) -C ext $@

.PHONY: again
again: clean
	$(MAKE)
