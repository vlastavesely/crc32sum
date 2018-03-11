/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __CRC32_H
#define __CRC32_H

unsigned int crc32_fd(int fd);
unsigned int crc32_file(const char *filename);

#endif /* __CRC32_H */
