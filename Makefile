.PHONY: all clean

all:
	$(CC) crc32.c -o crc32
	./crc32

clean:
	$(RM) -f crc32
