DESTDIR := /usr/local

.PHONY: install
install:
	@install -D -m 644 include/larena.h $(DESTDIR)/include/larena.h

.PHONY: test
test:
	@make -C tests all
