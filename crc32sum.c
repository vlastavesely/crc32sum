/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include "crc32.h"
#include "queue.h"

#include "crc32sum.h"

static const char *usage_str =
	"usage: " PROGNAME " [OPTION]... [FILE]...\n"
	"\n"
	"Print or check CRC32 checksums.\n"
	"\n"
	"With no FILE, read standard input.\n"
	"\n"
	"  -c, --check      read CRC32 sums from the FILE and check them\n"
	"  -r, --recursive  generate CRC32 sums for all files in given directories\n"
	"  -p, --progress   show a progressbar\n"
	"\n"
	"The following options are useful only when verifying checksums:\n"
	"  -q, --quiet      don't print OK for each successfully verified file\n"
	"  -s, --status     don't output anything, status code shows success\n"
	"\n"
	"      --help       display this help and exit\n"
	"      --version    output version information and exit\n";

static const char *short_opts = "hvc:qrps";

static const struct option long_opts[] = {
	{"help",       no_argument,        0, 'h'},
	{"version",    no_argument,        0, 'v'},
	{"check",      required_argument,  0, 'c'},
	{"quiet",      no_argument,        0, 'q'},
	{"recursive",  no_argument,        0, 'r'},
	{"progress",   no_argument,        0, 'p'},
	{"status",     no_argument,        0, 's'},
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

static void errno_to_error(int err, const char *path)
{
	switch (-err) {
	case ENOENT:
		error("'%s' not found.", path);
		break;
	case EACCES:
		error("do not have access to '%s'.");
		break;
	case EINVAL:
		error("'%s' is not regular file.", path);
		break;
	case EISDIR:
		error("'%s' is a directory.", path);
		break;
	default:
		error("cannot open '%s': error %d.", path, err);
	}
}

static int queue_compute_checksums(struct queue *queue, unsigned int flags)
{
	struct file *walk;
	struct progress *progress = NULL;
	unsigned int checksum;
	const char *path;
	int retval = 0;

	if (flags & CRC32SUM_PROGRESS)
		progress = progress_alloc(queue->nbytes);

	for (walk = queue->head; walk; walk = walk->next) {
		path = walk->path;
		checksum = crc32_file(path, progress);
		if (checksum == -1) {
			errno_to_error(-errno, path);
			retval++;
		} else {
			fprintf(stdout, "%08x  %s\n", checksum, path);
		}
	}

	if (progress)
		progress_drop(progress);

	return retval;
}

static int do_stdin_checksum()
{
	unsigned int checksum;
	int saved_errno;

	checksum = crc32_fd(STDIN_FILENO, NULL);
	if (checksum == -1) {
		saved_errno = errno;
		error("failed to read from stdin");
		return -saved_errno;
	}

	fprintf(stdout, "%08x\n", checksum);

	return 0;
}

static inline void trim_trailing_newlines(char *str)
{
	char *last = str + strlen(str) - 1;

	while (last > str && ((char) *last == '\n' || (char) *last == '\r'))
		*(last--) = '\0';
}

static int parse_sum_file(struct queue *queue, const char *filename)
{
	FILE *fp;
	char *line = NULL, *path;
	size_t len = 0;
	ssize_t n = 0;
	int retval;

	fp = fopen(filename, "r");
	if (fp == NULL)
		return -errno;

	while (1) {
		n = getline(&line, &len, fp);
		if (n == -1)
			break;
		if (n < 10)
			continue;

		path = line + 10;
		trim_trailing_newlines(path);
		retval = queue_schedule_regular_file(queue, path, (void *) strtol(line, NULL, 16));
		if (retval)
			errno_to_error(retval, path);
	}

	free(line);
	fclose(fp);

	return 0;
}

static int queue_do_checksums_check(struct queue *queue, unsigned int flags)
{
	struct file *walk;
	struct progress *progress = NULL;
	unsigned int sum;
	int retval = 0;

	if (flags & CRC32SUM_PROGRESS)
		progress = progress_alloc(queue->nbytes);

	for (walk = queue->head; walk; walk = walk->next) {
		sum = crc32_file(walk->path, progress);
		if (sum == -1) {
			error("failed to compute CRC of '%s'.", walk->path);
		} else if (sum != (unsigned long) walk->userdata) {
			if ((flags & CRC32SUM_STATUS) == 0)
				fprintf(stdout, "%s: FAILED\n", walk->path);
			retval++;
		} else if ((flags & (CRC32SUM_QUIET | CRC32SUM_STATUS)) == 0) {
			fprintf(stdout, "%s: OK\n", walk->path);
		}
	}

	if (progress)
		progress_drop(progress);

	return retval;
}

int main(int argc, char *const *argv)
{
	int c = 0, retval = 0;
	int opt_index = 0;
	unsigned int flags = 0;
	const char *check = NULL, *path = NULL;
	struct queue queue;

	while (c != -1) {
		c = getopt_long(argc, argv, short_opts, long_opts, &opt_index);

		switch (c) {
		case 'v':
			show_version();
			goto out;
		case 'h':
			show_usage();
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
		case 'p':
			flags |= CRC32SUM_PROGRESS;
			break;
		case 's':
			flags |= CRC32SUM_STATUS;
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

	crc32_initialize();

	queue_init(&queue);

	if (check) {
		retval = parse_sum_file(&queue, check);
		if (retval != 0) {
			error("failed to load '%s'", check);
			goto out;
		}

		retval = queue_do_checksums_check(&queue, flags);
		goto out;
	}

	if (optind == argc) {
		retval = do_stdin_checksum();
		goto out;
	}

	if (optind < argc) {
		while (optind < argc) {
			path = argv[optind++];
			retval = queue_schedule_path(&queue, path, flags);
			if (retval) {
				errno_to_error(retval, path);
				goto out;
			}
		}
	}

	retval = queue_compute_checksums(&queue, flags);

out:
	queue_clear(&queue);
	return retval;
}
