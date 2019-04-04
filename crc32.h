/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __CRC32_H
#define __CRC32_H

#include "progress.h"

void crc32_initialize(void);
unsigned int crc32_fd(int fd, struct progress *progress);
unsigned int crc32_file(const char *filename, struct progress *progress);

#endif /* __CRC32_H */
