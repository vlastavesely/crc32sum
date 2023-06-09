#include "test.h"
#include "integration.h"
#include "../config.h"

static char *exec_command(const char *command)
{
	char buf[512], *ret = malloc(1);
	FILE *proc;
	int n, len = 0;

	proc = popen(command, "r");

	while (1) {
		n = fread(buf, 1, sizeof(buf), proc);
		if (n == -1 || n == 0)
			break;

		ret = realloc(ret, len + n + 1);
		memcpy(ret + len, buf, n);
		len += n;
	}

	if (ret)
		ret[len] = '\0';

	pclose(proc);

	return ret;
}

static void test_output(const char *command, const char *expected)
{
	char *out;

	out = exec_command(command);
	ck_assert_str_eq(expected, out);
	free(out);
}

START_TEST(test_crc32_calculate)
{
	test_output("./crc32sum tests/files/a tests/files/b",
		"5ecfe3c5  tests/files/a\n6f27f958  tests/files/b\n");

	test_output("./crc32sum -r tests/files/dir | sort",
		"0cf7cc62  tests/files/dir/d\nc950f2ec  tests/files/dir/c\n");

	test_output("echo -n hello | ./crc32sum",
		"3610a686\n");
}
END_TEST

START_TEST(test_crc32_check)
{
	test_output("./crc32sum -q -c tests/files/badsums.txt",
		"tests/files/./a: FAILED\n");

	test_output("./crc32sum -s -c tests/files/badsums.txt",
		"");

	test_output("./crc32sum -c tests/files/sums.crlf.txt",
		"tests/files/./a: OK\n"
		"tests/files/./b: OK\n");

	test_output("./crc32sum -c tests/files/sums.lf.txt",
		"tests/files/./a: OK\n"
		"tests/files/./b: OK\n");

	test_output("cd tests && ../crc32sum -c files/sums.lf.txt",
		"files/./a: OK\n"
		"files/./b: OK\n");
}
END_TEST

START_TEST(test_crc32_progress)
{
	/* check that it does not fail */
	test_output("./crc32sum -p -c tests/files/sums.lf.txt 2>&1",
		"tests/files/./a: OK\n"
		"tests/files/./b: OK\n");
}
END_TEST

START_TEST(test_crc32_errors)
{
	test_output("./crc32sum tests/files/dir 2>&1",
		"crc32sum: 'tests/files/dir' is a directory.\n");
}
END_TEST

START_TEST(test_crc32_version)
{
	test_output("./crc32sum -v",
		"crc32sum v" PACKAGE_VERSION "\n");
}
END_TEST

void register_integration_tests(TCase *test_case)
{
	tcase_add_test(test_case, test_crc32_calculate);
	tcase_add_test(test_case, test_crc32_check);
	tcase_add_test(test_case, test_crc32_progress);
	tcase_add_test(test_case, test_crc32_errors);
	tcase_add_test(test_case, test_crc32_version);
}
