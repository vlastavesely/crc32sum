/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __CRC32_H
#define __CRC32_H

#include "progress.h"

/**
 * CRC32 checksum computation.
 *
 * WARNING: this implementation is optimized using precomputed lookup tables.
 * The tables are meant to be computed during runtime and it is user's
 * responsibility to call crc32_initialize() function manually. Alternatively,
 * ehen #CRC32_AUTOINIT macro is defined, initialization is run automatically.
 *
 * On success, functions crc32_fd() and crc32_file() return computed checksum,
 * 0 is returned on error, and errno is set appropriately.
 */

/* #define CRC32_AUTOINIT */

void crc32_initialize(void);
unsigned int crc32_fd(int fd, struct progress *progress);
unsigned int crc32_file(const char *filename, struct progress *progress);

#endif /* __CRC32_H */
