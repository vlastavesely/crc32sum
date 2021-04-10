SHELL  = /bin/sh
CC     = gcc

TARGET = crc32sum
CFLAGS = -Ofast -Wall

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man

SRCFILES = $(shell find . -type f -name "*.c")
OBJFILES = $(patsubst %.c, %.o, $(SRCFILES))


.PHONY: all install uninstall test clean

all: $(TARGET) $(TARGET).1.gz

include $(wildcard *.d tests/*.d)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

%-simd.o: %-simd.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@ -msse4.1 -mpclmul

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) $^ -o $(TARGET)

%.1.gz: %.1
	cat $< | gzip -f >$@

install:
	install -m 0755 $(TARGET) $(BINDIR)
	install -m 644 $(TARGET).1.gz $(MANDIR)/man1

uninstall:
	rm -rf $(BINDIR)/$(TARGET)
	rm -f $(MANDIR)/man1/$(TARGET).1.gz

test: $(TARGET)
	sh test.sh

clean:
	$(RM) $(TARGET) *.o *.d *.1.gz
