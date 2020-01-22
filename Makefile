PREFIX=/usr
TARGET=crc32sum
CFLAGS=-Ofast -Wall

SRCFILES := $(shell find . -type f -name "*.c")
OBJFILES := $(patsubst %.c, %.o, $(SRCFILES))

.PHONY: all install uninstall test clean

all: $(TARGET) doc/$(TARGET).1.gz

%.o: %.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) $^ -o $(TARGET)

doc/%.1.gz: doc/%.1.adoc
	asciidoctor -d manpage -b manpage $< -o $(<:.adoc=) && gzip -f $(<:.adoc=)

install:
	install -m 0755 $(TARGET) $(PREFIX)/bin/$(TARGET)
	install -m 644 doc/$(TARGET).1.gz $(PREFIX)/share/man/man1

uninstall:
	rm -rf $(PREFIX)/bin/$(TARGET)
	rm -f $(PREFIX)/share/man/man1/$(TARGET).1.gz

test: $(TARGET)
	sh test.sh

clean:
	$(RM) $(TARGET) *.o *.d doc/*.gz
