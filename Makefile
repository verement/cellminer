
ext/spu_miner.so: ext/Makefile ext/*.[ch] ext/spu/*.[ch]
	$(MAKE) -C ext

ext/Makefile: ext/extconf.rb ext/depend
	cd ext && ruby extconf.rb

.PHONY: clean
clean:
	$(MAKE) -C ext $@
