/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __UTILS_H
#define __UTILS_H

struct progress;

/**
 * On success, the functions crc32_fd() and crc32_file() return a positive
 * number representing the computed checksum. On error, a negative error code
 * is returned. The error codes correspond to errno(3).
 */
long crc32_fd(int fd, struct progress *progress);
long crc32_file(const char *filename, struct progress *progress);

#endif /* __UTILS_H */
