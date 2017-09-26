CC = gcc
CFLAGS ?= -O2 -g
CFLAGS += -Wall -Wextra

.PHONY: all
all: libfakenv.so

.PHONY: clean
clean:
	rm -f *.so

lib%.so: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -shared -fPIC -lrt -ldl $(<) -o $(@)

# vim:ts=2 sts=2 sw=2 noet
