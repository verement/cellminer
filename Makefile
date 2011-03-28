
RUBY	= ruby1.9.1

ext/spu_miner.so: ext/Makefile ext/*.[ch] ext/spu/*.[ch]
	$(MAKE) -C ext

ext/Makefile: ext/extconf.rb ext/depend
	cd ext && $(RUBY) extconf.rb
	$(MAKE) -C ext clean

.PHONY: clean
clean:
	$(MAKE) -C ext $@
