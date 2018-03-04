PREFIX=/usr
TARGET=crc32

.PHONY: all install uninstall test clean

all: $(TARGET)

$(TARGET): crc32.c
	$(CC) crc32.c -o $@ -Ofast

test:
	sh test.sh

install:
	install -m 0755 $(TARGET) $(PREFIX)/bin/$(TARGET)

uninstall:
	rm -r $(PREFIX)/bin/$(TARGET)

clean:
	$(RM) -f $(TARGET)
