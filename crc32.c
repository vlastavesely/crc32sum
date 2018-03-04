#include <stdio.h>
#include <stdlib.h>
#include <string.h>



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

	return 0;
}
