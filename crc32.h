/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __CRC32_H
#define __CRC32_H

/**
 * CRC32 checksum computation.
 *
 * WARNING: this implementation is optimised using precomputed lookup tables.
 * The tables are meant to be computed during the runtime and it is the user's
 * responsibility to call the crc32_initialise() function manually.
 */

void crc32_initialise(void);
unsigned int crc32_buffer(const unsigned char *buf, unsigned int len,
			  unsigned int crc);

#endif /* __CRC32_H */
