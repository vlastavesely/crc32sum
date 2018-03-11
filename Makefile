PREFIX=/usr
TARGET=crc32sum
CFLAGS=-Ofast

SRCFILES := $(shell find . -type f -name "*.c")
OBJFILES := $(patsubst %.c, %.o, $(SRCFILES))

.PHONY: all install uninstall test clean

all: $(TARGET)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) $^ -o $(TARGET)

install:
	install -m 0755 $(TARGET) $(PREFIX)/bin/$(TARGET)

uninstall:
	rm -r $(PREFIX)/bin/$(TARGET)

test: $(TARGET)
	sh test.sh

clean:
	$(RM) $(TARGET) *.o *.d
