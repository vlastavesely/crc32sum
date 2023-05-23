#include "test.h"
#include "crc32.h"
#include "queue.h"
#include "integration.h"
#include "../crc32.h" /* crc32_initialise() */

static Suite *create_test_suite()
{
	Suite *suite;
	TCase *test_case;

	suite = suite_create(NULL);
	test_case = tcase_create(NULL);

	register_crc32_tests(test_case);
	register_queue_tests(test_case);
	register_integration_tests(test_case);

	suite_add_tcase(suite, test_case);

	return suite;
}

int main(void)
{
	Suite *suite;
	SRunner *runner;
	int retval;

	suite = create_test_suite();
	runner = srunner_create(suite);

	crc32_initialise();

	puts("-----------------------------------------");
	srunner_run_all(runner, CK_NORMAL);
	retval = srunner_ntests_failed(runner);
	puts("-----------------------------------------");

	puts(retval == 0 ? "\033[32mpassed\033[0m\n"
			 : "\033[31mfailed\033[0m\n");

	srunner_free(runner);

	return retval;
}

