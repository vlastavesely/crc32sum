#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>

#define PROGNAME "crc32"
#define VERSION "0.1"
#define BUFSIZE 4096

static const char *short_opts = "hvc:";

static const struct option long_opts[] = {
	{"help",       no_argument,        0, 'h'},
	{"version",    no_argument,        0, 'v'},
	{"check",      required_argument,  0, 'c'},
	{0, 0, 0, 0}
};

static void show_usage()
{
	puts("*usage*"); /* TODO */
}

static void show_version()
{
	puts(PROGNAME " v" VERSION);
}

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

int do_file_checksum(const char *filename)
{
	printf("file: %s\n", filename);
	/* TODO */
	return 0;
}

int do_check(const char *filename)
{
	/* TODO */
}

int main(int argc, char *const *argv)
{
	int c = 0, retval = 0;
	int opt_index = 0;
	const unsigned char *check = NULL;

	while (c != -1) {
		c = getopt_long(argc, argv, short_opts, long_opts, &opt_index);

		switch (c) {
		case 'v':
			show_version();
			retval = 0;
			goto out;
		case 'h':
			show_usage();
			retval = 0;
			goto out;
		case 'c':
			check = optarg;
			break;
		case 0:
		case '?':
			show_usage();
			retval = 1;
			goto out;
		default:
			break;
		}
	}

	if (check) {
		retval = do_check(check);
		goto out;
	}

	if (optind < argc) {
		while (optind < argc)
			if (do_file_checksum(argv[optind++]) != 0)
				retval++;
	}

out:
	return retval;
}
