#include "test.h"
#include "queue.h"
#include "../compat.h"
#include "../queue.h"
#include "../error.h"
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

static void empty_error_handler(const char *message)
{
}

START_TEST(test_queue)
{
	struct queue queue = {};
	int ret, i;

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

#define BUILD_FILENAME(b, d, f) {		\
	snprintf(b, sizeof(b), "%s/" f, d);	\
}

#define REMOVE_FILE(b, d, f) {			\
	BUILD_FILENAME(b, d, f);		\
	remove(b);				\
}

START_TEST(test_queue_symlinks)
{
	struct queue queue = {};
	char tmpdir[] = "/tmp/crc32test.XXXXXX";
	char buf[64];
	int ret;

	set_error_handler(empty_error_handler);
	mkdtemp(tmpdir);

	BUILD_FILENAME(buf, tmpdir, "dir");
	mkdir(buf, 0755);

	BUILD_FILENAME(buf, tmpdir, "dir/file");
	close(open(buf, O_WRONLY | O_CREAT | O_TRUNC, 0644));

	BUILD_FILENAME(buf, tmpdir, "link");
	symlink("dir", buf);

	ret = queue_schedule_path(&queue, tmpdir, CRC32SUM_RECURSIVE | CRC32SUM_FOLLOW);
	ck_assert_int_eq(0, ret);

	sort_queue_paths(&queue);
	ck_assert_int_eq(2, queue.nfiles);
	ck_assert_str_eq("dir/file", queue.files[0].path + strlen(tmpdir) + 1);
	ck_assert_str_eq("link/file", queue.files[1].path + strlen(tmpdir) + 1);

	REMOVE_FILE(buf, tmpdir, "dir/file");
	REMOVE_FILE(buf, tmpdir, "dir");
	REMOVE_FILE(buf, tmpdir, "link");

	rmdir(tmpdir);
}

START_TEST(test_queue_error_isdir)
{
	struct queue queue = {};

	set_error_handler(empty_error_handler);
	ck_assert_int_eq(-EISDIR, queue_schedule_path(&queue, "tests", 0));
	queue_clear(&queue);
}
END_TEST

START_TEST(test_queue_error_symlinks)
{
	struct queue queue = {};
	char tmpdir[] = "/tmp/crc32test.XXXXXX";
	char link_name[64];

	set_error_handler(empty_error_handler);
	mkdtemp(tmpdir);

	snprintf(link_name, sizeof(link_name), "%s/link", tmpdir);

	/* a broken link that points to non-existent destination */
	symlink("foo", link_name);
	ck_assert_int_eq(-ENOLINK, queue_schedule_path(&queue, link_name, 0));
	queue_clear(&queue);

	remove(link_name);

	/* when a link points to its parent directories, there is a loop */
	symlink("..", link_name);
	ck_assert_int_eq(-ELOOP, queue_schedule_path(&queue, link_name, CRC32SUM_FOLLOW));
	queue_clear(&queue);
	remove(link_name);

	symlink("../..", link_name);
	ck_assert_int_eq(-ELOOP, queue_schedule_path(&queue, link_name, CRC32SUM_FOLLOW));
	queue_clear(&queue);
	remove(link_name);

	rmdir(tmpdir);
}

void register_queue_tests(TCase *test_case)
{
	tcase_add_test(test_case, test_queue);
	tcase_add_test(test_case, test_queue_symlinks);
	tcase_add_test(test_case, test_queue_error_isdir);
	tcase_add_test(test_case, test_queue_error_symlinks);
}
