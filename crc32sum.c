/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include "crc32.h"
#include "file-list.h"

#define PROGNAME "crc32sum"
#define VERSION "0.1"

#define CRC32SUM_QUIET		1 << 0
#define CRC32SUM_RECURSIVE	1 << 1

static const char *usage_str =
	"usage: " PROGNAME " [OPTION]... [FILE]...\n"
	"\n"
	"Print or check CRC32 checksums.\n"
	"\n"
	"With no FILE, read standard input.\n"
	"\n"
	"  -c, --check      read CRC32 sums from the FILE and check them\n"
	"  -r, --recursive  generate CRC32 sums for all files in given directories\n"
	"\n"
	"The following options are useful only when verifying checksums:\n"
	"      --quiet      don't print any output\n"
	"\n"
	"      --help       display this help and exit\n"
	"      --version    output version information and exit\n";

static const char *short_opts = "hvc:qr";

static const struct option long_opts[] = {
	{"help",       no_argument,        0, 'h'},
	{"version",    no_argument,        0, 'v'},
	{"check",      required_argument,  0, 'c'},
	{"quiet",      no_argument,        0, 'q'},
	{"recursive",  no_argument,        0, 'r'},
	{0, 0, 0, 0}
};

static void show_usage()
{
	fprintf(stdout, "%s\n", usage_str);
}

static void show_version()
{
	puts(PROGNAME " v" VERSION);
}

#undef error
void error(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	fprintf(stderr, PROGNAME ": ");
	vfprintf(stderr, err, params);
	fputc('\n', stderr);
	va_end(params);
}

static int do_file_checksum(const char *filename)
{
	unsigned int checksum;

	checksum = crc32_file(filename);
	if (checksum == 0 && errno) {
		switch (errno) {
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
			error("cannot open '%s': error %d.", filename, errno);
			break;
		}
		return -errno;
	}

	fprintf(stdout, "%08x  %s\n", checksum, filename);

	return 0;
}

static int do_file_checksum_recursively(const char *filename)
{
	struct file *head, *walk;
	int retval = 0;

	head = file_list_create(filename);

	for (walk = head; walk; walk = walk->next) {
		if (walk->type == FILE_TYPE_FILE) {
			retval = do_file_checksum(walk->path);
			if (retval != 0)
				goto drop_list;
		}
	}

drop_list:
	file_list_drop(head);

	return retval;
}

static int do_stdin_checksum()
{
	unsigned int checksum;
	int saved_errno;

	checksum = crc32_fd(STDIN_FILENO);
	if (checksum == 0 && errno) {
		saved_errno = errno;
		error("failed to read from stdin");
		return -saved_errno;
	}

	fprintf(stdout, "%08x\n", checksum);

	return 0;
}

static void trim_trailing_newlines(char *str)
{
	char *last = str + strlen(str) - 1;

	while (last > str && ((char) *last == '\n' || (char) *last == '\r'))
		*(last--) = '\0';
}

static int do_check(const char *filename, unsigned int flags)
{
	FILE *fp;
	char *line = NULL, *path;
	size_t len = 0;
	ssize_t n = 0;
	unsigned int checksum, checksum2;
	int retval = 0, failed = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		error("failed to open '%s'.", filename);
		return 1;
	}

	while ((n = getline(&line, &len, fp)) != -1) {
		if (n < 10)
			continue;

		path = line + 10;
		trim_trailing_newlines(path);

		checksum = crc32_file(path);

		if (checksum == 0 && errno) {
			retval = errno;
			error("failed to compute CRC of '%s'.", path);
			failed++;
		} else {
			checksum2 = strtol(line, NULL, 16);

			if (checksum != checksum2)
				failed++;

			if (flags & CRC32SUM_QUIET)
				continue;

			fprintf(stdout, "%s: %s\n", path,
				checksum == checksum2 ? "OK" : "FAILED");
		}
	}

	if (failed)
		fprintf(stdout, "WARNING: %d computed checksum(s) did "
				"NOT match\n", failed);

	free(line);
	fclose(fp);

	return retval | failed;
}

int main(int argc, char *const *argv)
{
	int c = 0, retval = 0;
	int opt_index = 0;
	unsigned int flags = 0;
	const char *check = NULL;

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
		case 'q':
			flags |= CRC32SUM_QUIET;
			break;
		case 'r':
			flags |= CRC32SUM_RECURSIVE;
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
		retval = do_check(check, flags);
		goto out;
	}

	if (optind < argc) {
		while (optind < argc) {
			if (flags & CRC32SUM_RECURSIVE) {
				if (do_file_checksum_recursively(argv[optind++]) != 0)
					retval++;
			} else {
				if (do_file_checksum(argv[optind++]) != 0)
					retval++;
			}
		}
	} else {
		retval = do_stdin_checksum();
	}

out:
	return retval;
}
