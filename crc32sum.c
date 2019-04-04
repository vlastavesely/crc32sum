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
	"      --quiet      don't print any output\n"
	"\n"
	"      --help       display this help and exit\n"
	"      --version    output version information and exit\n";

static const char *short_opts = "hvc:qrp";

static const struct option long_opts[] = {
	{"help",       no_argument,        0, 'h'},
	{"version",    no_argument,        0, 'v'},
	{"check",      required_argument,  0, 'c'},
	{"quiet",      no_argument,        0, 'q'},
	{"recursive",  no_argument,        0, 'r'},
	{"progress",   no_argument,        0, 'p'},
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

static int do_file_checksum(const char *filename, struct progress *progress)
{
	unsigned int checksum;

	checksum = crc32_file(filename, progress);
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

static int do_stdin_checksum()
{
	unsigned int checksum;
	int saved_errno;

	checksum = crc32_fd(STDIN_FILENO, NULL);
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

static int do_file_checksum_check(const char *path, unsigned int expected,
				  struct progress *progress)
{
	unsigned int checksum;
	int retval = 0;

	checksum = crc32_file(path, progress);

	if (checksum == 0 && errno) {
		retval = errno;
		error("failed to compute CRC of '%s'.", path);
		return -1;
	}

	return checksum == expected ? 0 : -1;
}

static int queue_do_checksums_check(struct queue *queue, unsigned int flags)
{
	struct file *walk;
	struct progress *progress = NULL;
	int retval = 0;

	if (flags & CRC32SUM_PROGRESS) {
		progress = malloc(sizeof(*progress));
		progress->max = queue->nbytes;
		progress->pos = 0;
		progress->add = progress_add;
		progress_start();
	}

	for (walk = queue->head; walk; walk = walk->next) {
		if (do_file_checksum_check(walk->path,
					   (unsigned int) walk->userdata,
					   progress) != 0) {
			retval++;
			if ((flags & CRC32SUM_QUIET) == 0)
				fprintf(stdout, "%s: FAILED\n", walk->path);
			continue;
		}

		fprintf(stdout, "%s: OK\n", walk->path);
	}

	if (progress) {
		progress_stop();
		free(progress);
	}

	return retval;
}

static int do_check(const char *filename, unsigned int flags)
{
	FILE *fp;
	char *line = NULL, *path;
	size_t len = 0;
	ssize_t n = 0;
	unsigned int checksum;
	int retval = 0;
	struct queue queue;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		error("failed to open '%s'.", filename);
		return 1;
	}

	queue_init(&queue);

	while (1) {
		n = getline(&line, &len, fp);
		if (n == -1)
			break;
		if (n < 10)
			continue;

		path = line + 10;
		trim_trailing_newlines(path);

		queue_schedule_regular_file(&queue, path, (void *) strtol(line, NULL, 16));
	}

	retval = queue_do_checksums_check(&queue, flags);

	queue_clear(&queue);
	free(line);
	fclose(fp);

	return retval;
}

static int queue_do_checksums(struct queue *queue, unsigned int flags)
{
	struct file *walk;
	struct progress *progress = NULL;
	int retval = 0;

	if (flags & CRC32SUM_PROGRESS) {
		progress = malloc(sizeof(*progress));
		progress->max = queue->nbytes;
		progress->pos = 0;
		progress->add = progress_add;
		progress_start();
	}

	for (walk = queue->head; walk; walk = walk->next)
		retval += do_file_checksum(walk->path, progress);

	if (progress) {
		progress_stop();
		free(progress);
	}

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
		case 'p':
			flags |= CRC32SUM_PROGRESS;
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

	if (check) {
		retval = do_check(check, flags);
		goto out;
	}

	if (optind == argc) {
		retval = do_stdin_checksum();
		goto out;
	}

	queue_init(&queue);

	if (optind < argc) {
		while (optind < argc) {
			path = argv[optind++];
			retval = queue_schedule_path(&queue, path, flags);

			switch (-retval) {
			case 0:
				break;
			case EISDIR:
				error("'%s' is a directory.", path);
				goto out;
			default:
				queue_clear(&queue);
				goto out;
			}
		}
	}

	queue_do_checksums(&queue, flags);

out:
	return retval;
}
