/* SPDX-License-Identifier: GPL-2.0 */

#include "compat.h"
#include "crc32sum.h"
#include "queue.h"

static int queue_schedule_file(struct queue *queue, const char *path,
				unsigned long size, unsigned int sum)
{
	struct file *file;

	queue->files = realloc(queue->files, (queue->nfiles + 1) * sizeof(*file));
	file = &queue->files[queue->nfiles++];

	file->path = strdup(path);
	file->size = size;
	file->sum = sum;
	queue->nbytes += size;

	return 0;
}

static inline int is_dot_or_dotdot(const char *name)
{
	return name[0] == '.' && (name[1] == '\0' ||
		(name[1] == '.' && name[2] == '\0'));
}

void queue_init(struct queue *queue)
{
	queue->files = NULL;
	queue->nfiles = 0;
	queue->nbytes = 0;
}

void queue_clear(struct queue *queue)
{
	free(queue->files);
	queue_init(queue);
}

int queue_schedule_regular_file(struct queue *queue, const char *path,
				unsigned int sum)
{
	struct stat sb;

	if (stat(path, &sb) != 0)
		return -errno;

	if (S_ISREG(sb.st_mode) == 0)
		return -EINVAL;

	return queue_schedule_file(queue, path, sb.st_size, sum);
}

int queue_schedule_path(struct queue *queue, const char *path,
			unsigned int flags)
{
	DIR *dir;
	struct dirent *de;
	struct stat sb;
	char *child;
	unsigned int len;
	int retval = 0;

	if (stat(path, &sb) != 0)
		return -errno;

	if (S_ISDIR(sb.st_mode)) {
		if ((flags & CRC32SUM_RECURSIVE) == 0)
			return -EISDIR;

		dir = opendir(path);
		if (dir == NULL)
			return -errno;

		while (1) {
			de = readdir(dir);
			if (de == NULL)
				break;
			if (is_dot_or_dotdot(de->d_name))
				continue;

			len = strlen(path) + strlen(de->d_name) + 3;
			child = malloc(len);
			snprintf(child, len, "%s/%s", path, de->d_name);

			retval = queue_schedule_path(queue, child, flags);
			free(child);
			if (retval != 0)
				goto close_dir;
		}
close_dir:
		closedir(dir);
	} else if (S_ISREG(sb.st_mode)) {
		queue_schedule_file(queue, path, sb.st_size, 0);
	} else {
		return -EINVAL;
	}

	return retval;
}
