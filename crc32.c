#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFSIZE 4096

static unsigned int crc32(unsigned char *data, unsigned int nbytes,
			  unsigned int crc)
{
	unsigned int mask, i = 0;
	int j;

	crc = ~crc;
	while (i < nbytes) {
		crc = crc ^ data[i];
		for (j = 7; j >= 0; j--) {
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xedb88320 & mask);
		}
		i++;
	}

	return ~crc;
}

static unsigned int crc32_file(const char *filename)
{
	unsigned int n, crc = 0;
	unsigned char buffer[BUFSIZE];
	int fd = open(filename, O_RDONLY);

	if (fd < 0)
		return 0;

	while ((n = read(fd, buffer, sizeof(buffer))) > 0)
		crc = crc32(buffer, n, crc);

	close(fd);

	return crc;
}

static void print_crc32(unsigned int crc)
{
	printf("%08x\n", crc);
}


int main(int argc, const char **argv)
{
	unsigned int crc;

	crc = crc32("hello", 5, 0);
	print_crc32(crc);
	puts("--------------------");

	crc = crc32("world", 5, 0);
	print_crc32(crc);
	puts("--------------------");

	crc = crc32("he", 2, 0);
	crc = crc32("llo", 3, crc);
	print_crc32(crc);
	puts("--------------------");

	crc = crc32_file("zeroes");
	print_crc32(crc);

	return 0;
}
