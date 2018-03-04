.PHONY: all test clean

all:
	$(CC) crc32.c -o crc32 -Ofast

test:
	sh test.sh

clean:
	$(RM) -f crc32
