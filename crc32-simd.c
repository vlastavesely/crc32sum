/*
 * Copyright 2017 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the Chromium source repository LICENSE file.
 *
 * Source: https://github.com/chromium/chromium/blob/master/third_party/zlib/crc32_simd.c
 */

#include <emmintrin.h>
#include <smmintrin.h>
#include <wmmintrin.h>
#include <stdint.h>
#include "crc32.h"

#ifndef ALIGN
#define ALIGN(n) __attribute__((aligned(n)))
#endif

static const uint64_t ALIGN(16) k1k2[] = {0x0154442bd4, 0x01c6e41596};
static const uint64_t ALIGN(16) k3k4[] = {0x01751997d0, 0x00ccaa009e};
static const uint64_t ALIGN(16) k5k0[] = {0x0163cd6124, 0x0000000000};
static const uint64_t ALIGN(16) poly[] = {0x01db710641, 0x01f7011641};

unsigned int crc32_buffer_simd(const unsigned char *buf, unsigned int len,
			       unsigned int crc)
{
	__m128i x0, x1, x2, x3, x4, x5, x6, x7, x8, y5, y6, y7, y8;

	/*
	 * There is at least one block of 64.
	 */
	x1 = _mm_loadu_si128((__m128i *) (buf + 0x00));
	x2 = _mm_loadu_si128((__m128i *) (buf + 0x10));
	x3 = _mm_loadu_si128((__m128i *) (buf + 0x20));
	x4 = _mm_loadu_si128((__m128i *) (buf + 0x30));

	x1 = _mm_xor_si128(x1, _mm_cvtsi32_si128(crc));

	x0 = _mm_load_si128((__m128i *) k1k2);

	buf += 64;
	len -= 64;

	/*
	 * Parallel fold blocks of 64, if any.
	 */
	while (len >= 64) {
		x5 = _mm_clmulepi64_si128(x1, x0, 0x00);
		x6 = _mm_clmulepi64_si128(x2, x0, 0x00);
		x7 = _mm_clmulepi64_si128(x3, x0, 0x00);
		x8 = _mm_clmulepi64_si128(x4, x0, 0x00);

		x1 = _mm_clmulepi64_si128(x1, x0, 0x11);
		x2 = _mm_clmulepi64_si128(x2, x0, 0x11);
		x3 = _mm_clmulepi64_si128(x3, x0, 0x11);
		x4 = _mm_clmulepi64_si128(x4, x0, 0x11);

		y5 = _mm_loadu_si128((__m128i *) (buf + 0x00));
		y6 = _mm_loadu_si128((__m128i *) (buf + 0x10));
		y7 = _mm_loadu_si128((__m128i *) (buf + 0x20));
		y8 = _mm_loadu_si128((__m128i *) (buf + 0x30));

		x1 = _mm_xor_si128(x1, x5);
		x2 = _mm_xor_si128(x2, x6);
		x3 = _mm_xor_si128(x3, x7);
		x4 = _mm_xor_si128(x4, x8);

		x1 = _mm_xor_si128(x1, y5);
		x2 = _mm_xor_si128(x2, y6);
		x3 = _mm_xor_si128(x3, y7);
		x4 = _mm_xor_si128(x4, y8);

		buf += 64;
		len -= 64;
	}

	/*
	 * Fold into 128-bits.
	 */
	x0 = _mm_load_si128((__m128i *) k3k4);

	x5 = _mm_clmulepi64_si128(x1, x0, 0x00);
	x1 = _mm_clmulepi64_si128(x1, x0, 0x11);
	x1 = _mm_xor_si128(x1, x2);
	x1 = _mm_xor_si128(x1, x5);

	x5 = _mm_clmulepi64_si128(x1, x0, 0x00);
	x1 = _mm_clmulepi64_si128(x1, x0, 0x11);
	x1 = _mm_xor_si128(x1, x3);
	x1 = _mm_xor_si128(x1, x5);

	x5 = _mm_clmulepi64_si128(x1, x0, 0x00);
	x1 = _mm_clmulepi64_si128(x1, x0, 0x11);
	x1 = _mm_xor_si128(x1, x4);
	x1 = _mm_xor_si128(x1, x5);

	/*
	 * Single fold blocks of 16, if any.
	 */
	while (len >= 16) {
		x2 = _mm_loadu_si128((__m128i *) buf);

		x5 = _mm_clmulepi64_si128(x1, x0, 0x00);
		x1 = _mm_clmulepi64_si128(x1, x0, 0x11);
		x1 = _mm_xor_si128(x1, x2);
		x1 = _mm_xor_si128(x1, x5);

		buf += 16;
		len -= 16;
	}

	/*
	 * Fold 128-bits to 64-bits.
	 */
	x2 = _mm_clmulepi64_si128(x1, x0, 0x10);
	x3 = _mm_setr_epi32(~0, 0, ~0, 0);
	x1 = _mm_srli_si128(x1, 8);
	x1 = _mm_xor_si128(x1, x2);

	x0 = _mm_loadl_epi64((__m128i*) k5k0);

	x2 = _mm_srli_si128(x1, 4);
	x1 = _mm_and_si128(x1, x3);
	x1 = _mm_clmulepi64_si128(x1, x0, 0x00);
	x1 = _mm_xor_si128(x1, x2);

	/*
	 * Barret reduce to 32-bits.
	 */
	x0 = _mm_load_si128((__m128i*) poly);

	x2 = _mm_and_si128(x1, x3);
	x2 = _mm_clmulepi64_si128(x2, x0, 0x10);
	x2 = _mm_and_si128(x2, x3);
	x2 = _mm_clmulepi64_si128(x2, x0, 0x00);
	x1 = _mm_xor_si128(x1, x2);

	crc = _mm_extract_epi32(x1, 1);

	if (len > 0)
		crc = crc32_buffer(buf, len, crc);

	return crc;
}
