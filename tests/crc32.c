#include "test.h"
#include "crc32.h"
#include "../compat.h"
#include "../crc32.h"

#ifdef HAVE_SSE4_1_INSTRUCTIONS
#include "../crc32-simd.h"
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

#define sixteen_a "aaaaaaaaaaaaaaaa"
#define thirtytwo_a sixteen_a sixteen_a

struct test_vector {
	const char *sum;
	const char *str;
};

static const struct test_vector test_vectors[] = {
	{"00000000", ""},

	/* test vectors are from the PHP ‘hash()’ function */
	{"e8b7be43", "a"},
	{"f007732d", "aaa"},
	{"ad98e545", "aaaa"},
	{"cfd668d5", sixteen_a},
	{"cab11777", thirtytwo_a},
	{"89b46555", thirtytwo_a thirtytwo_a},
	{"6ebcf710", thirtytwo_a thirtytwo_a thirtytwo_a},

	/* test vectors generated using ‘zlib.crc32()’ in python3 */
	{"414fa339", "The quick brown fox jumps over the lazy dog"},
	{"519025e9", "The quick brown fox jumps over the lazy dog."}
};

START_TEST(test_crc32)
{
	unsigned int sum, i;
	const char *data;
	char buf[9] = {};

	for (i = 0; i < ARRAY_SIZE(test_vectors); i++) {
		data = test_vectors[i].str;

		sum = ~crc32_buffer((unsigned char *) data, strlen(data), ~0);
		snprintf(buf, 9, "%08x", sum);
		ck_assert_str_eq(test_vectors[i].sum, buf);
	}
}
END_TEST

#ifdef HAVE_SSE4_1_INSTRUCTIONS
START_TEST(test_crc32_simd)
{
	unsigned int sum, i;
	const char *data;
	char buf[9] = {};

	for (i = 0; i < ARRAY_SIZE(test_vectors); i++) {
		data = test_vectors[i].str;

		sum = ~crc32_buffer_simd((unsigned char *) data, strlen(data), ~0);
		snprintf(buf, 9, "%08x", sum);
		ck_assert_str_eq(test_vectors[i].sum, buf);
	}
}
END_TEST
#endif

void register_crc32_tests(TCase *test_case)
{
	tcase_add_test(test_case, test_crc32);
	#ifdef HAVE_SSE4_1_INSTRUCTIONS
	tcase_add_test(test_case, test_crc32_simd);
	#endif
}
