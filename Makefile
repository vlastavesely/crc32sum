PREFIX=/usr
TARGET=crc32sum

.PHONY: all install uninstall test clean

all: $(TARGET)

$(TARGET): crc32sum.c
	$(CC) crc32sum.c -o $@ -Ofast

test:
	sh test.sh

install:
	install -m 0755 $(TARGET) $(PREFIX)/bin/$(TARGET)

uninstall:
	rm -r $(PREFIX)/bin/$(TARGET)

clean:
	$(RM) -f $(TARGET)
