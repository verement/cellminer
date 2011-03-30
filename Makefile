
RUBY	= ruby1.9.1

ext/spu_miner.so: ext/Makefile ext/*.[ch] ext/spu/Makefile ext/spu/*.[chs]
	$(MAKE) -C ext

ext/Makefile: ext/extconf.rb ext/depend
	cd ext && $(RUBY) extconf.rb

.PHONY: clean
clean:
	$(MAKE) -C ext $@

.PHONY: again
again: clean
	$(MAKE)
