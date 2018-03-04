#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "err.h"

#define PROGNAME "crc32"
#define VERSION "0.1"
#define BUFSIZE 4096

struct crc32_checksum {
	const char *filename;
	unsigned int crc;
};

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

#undef error
void error(const char *err, ...)
{
	va_list params;
	char msg[4096];

	va_start(params, err);
	fprintf(stderr, PROGNAME ": ");
	vfprintf(stderr, err, params);
	fprintf(stderr, "\n");
	va_end(params);
}

static unsigned int crc32_buffer(unsigned char *data, unsigned int nbytes,
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

struct crc32_checksum *crc32_file(const char *filename)
{
	struct crc32_checksum *cksum;
	struct stat st;
	unsigned char buffer[BUFSIZE];
	unsigned int crc = 0;
	int fd, n = 0;

	if (stat(filename, &st) != 0)
		return ERR_PTR(-errno);

	if (S_ISDIR(st.st_mode))
		return ERR_PTR(-EISDIR);

	cksum = malloc(sizeof(*cksum));
	if (cksum == NULL)
		return ERR_PTR(-ENOMEM);

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return ERR_PTR(-errno);

	while ((n = read(fd, buffer, sizeof(buffer))) > 0)
		crc = crc32_buffer(buffer, n, crc);
	close(fd);

	cksum->filename = filename;
	cksum->crc = crc;
	return cksum;
}

static int do_file_checksum(const char *filename)
{
	struct crc32_checksum *cksum;
	int err;

	cksum = crc32_file(filename);

	if (IS_ERR(cksum)) {
		err = PTR_ERR(cksum);
		switch (-err) {
		case ENOENT:
			error("'%s' not found.", filename);
			break;
		case EISDIR:
			error("'%s' is a directory.", filename);
			break;
		case EACCES:
			error("do not have access to '%s'.", filename);
			break;
		default:
			error("cannot open '%s': error %d.", filename, err);
			break;
		}
		return err;
	}

	fprintf(stdout, "%08x  %s\n", cksum->crc, cksum->filename);
	free(cksum);

	return 0;
}

static void trim_trailing_newlines(char *str)
{
	char *last = str + strlen(str) - 1;

	while (last > str && ((char) *last == '\n' || (char) *last == '\r'))
		*(last--) = '\0';
}

static int do_check(const char *filename)
{
	struct crc32_checksum *cksum;
	FILE *fp;
	char *line = NULL, *path;
	size_t len = 0;
	ssize_t n = 0;
	unsigned int crc;
	int retval = 0, failed = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		error("failed to open '%s'.", filename);
		return 1;
	}

	while ((n = getline(&line, &len, fp)) != -1) {
		path = line + 10;
		trim_trailing_newlines(path);
		cksum = crc32_file(path);
		if (IS_ERR(cksum)) {
			error("failed to compute CRC of '%s'.", path);
			retval = PTR_ERR(cksum);
			failed++;
		} else {
			fprintf(stdout, "%s: ", path);
			crc = strtol(line, NULL, 16);
			if (cksum->crc == crc) {
				fprintf(stdout, "OK\n");
			} else {
				fprintf(stdout, "FAILED\n");
				failed++;
			}
		}
	}

	if (failed)
		fprintf(stdout, "WARNING: %d computed checksum(s) did "
				"NOT match\n", failed);

	free(line);
	fclose(fp);

	return retval;
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
