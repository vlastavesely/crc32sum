#include "test.h"
#include "queue.h"
#include "../queue.h"
#include "../crc32sum.h"

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(*a))

/*
 * Sorts just the paths, not the sizes and other details!!
 */
static void sort_queue_paths(struct queue *queue)
{
	unsigned int i, j;

	for (i = 0; i < queue->nfiles; i++) {
		struct file *a = &(queue->files[i]);
		for (j = 0; j < queue->nfiles; j++) {
			struct file *b = &(queue->files[j]);
			if (strcmp(a->path, b->path) < 0) {
				char *tmp = b->path;
				b->path = a->path;
				a->path = tmp;
			}
		}
	}
}

START_TEST(test_queue)
{
	struct queue queue;
	int ret, i;

	queue_init(&queue);

	ret = queue_schedule_path(&queue, "tests/files", CRC32SUM_RECURSIVE);
	ck_assert_int_eq(0, ret);

	const char *expected[] = {
		"tests/files/a",
		"tests/files/b",
		"tests/files/badsums.txt",
		"tests/files/dir/c",
		"tests/files/dir/d",
		"tests/files/sums.crlf.txt",
		"tests/files/sums.lf.txt"
	};

	ck_assert_int_eq(ARRAY_COUNT(expected), queue.nfiles);

	/* readdir() does not garantee order... */
	sort_queue_paths(&queue);

	for (i = 0; i < queue.nfiles; i++) {
		struct file *file = &(queue.files[i]);
		ck_assert_str_eq(expected[i], file->path);
	}

	queue_clear(&queue);
}
END_TEST

void register_queue_tests(TCase *test_case)
{
	tcase_add_test(test_case, test_queue);
}
