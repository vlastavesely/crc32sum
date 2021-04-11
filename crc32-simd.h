/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __CRC32_SIMD_H
#define __CRC32_SIMD_H

/**
 * CRC32 checksum computation (SSE4.1 optimised).
 *
 * WARNING: this implementation requires to initialise the lookup tables by
 * calling the crc32_initialise() function manually. It is the user's
 * responsibility to do that.
 */

unsigned int crc32_buffer_simd(const unsigned char *buf, unsigned int len,
			       unsigned int crc);

#endif /* __CRC32_SIMD_H */
