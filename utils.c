#include "compat.h"
#include "utils.h"
#include "progress.h"

#ifdef HAVE_SSE4_1_INSTRUCTIONS
#include "crc32-simd.h"
#define CRC32_PROCESS crc32_buffer_simd
#else
#include "crc32.h"
#define CRC32_PROCESS crc32_buffer
#endif

#define BUFSIZE (1 << 25) /* 2^25 = 32M */

long crc32_fd(int fd, struct progress *progress)
{
	unsigned char *buffer, *ptr;
	unsigned int align, checksum = ~0;
	int n = 0;

	buffer = malloc(BUFSIZE + 16);
	align = (unsigned long) buffer % 16;
	ptr = buffer + (16 - align);

	while (1) {
		n = read(fd, ptr, BUFSIZE);
		if (n == -1) {
			checksum = -errno;
			goto out;
		}
		if (n == 0)
			break;

		checksum = CRC32_PROCESS(ptr, n, checksum);

		if (progress)
			progress->add(progress, n);
	}

	checksum = ~checksum;
out:
	free(buffer);

	return checksum;
}

long crc32_file(const char *filename, struct progress *progress)
{
	struct stat st;
	unsigned int checksum;
	int fd;

	if (stat(filename, &st) != 0)
		return -errno;

	if (S_ISDIR(st.st_mode))
		return -EISDIR;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return -errno;

	checksum = crc32_fd(fd, progress);
	close(fd);

	return checksum;
}
