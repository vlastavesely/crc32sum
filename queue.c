/* SPDX-License-Identifier: GPL-2.0 */

#include "compat.h"
#include "crc32sum.h"
#include "queue.h"
#include "error.h"

struct schedule_status {
	char *path;
	int error;
};

static int schedule_file(struct queue *queue, const char *path,
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

	return schedule_file(queue, path, sb.st_size, sum);
}

static inline void set_error(struct schedule_status *status, const char *path,
			     int code)
{
	status->path = strdup(path);
	status->error = code;
}

static inline bool has_flag(unsigned int flags, unsigned int flag)
{
	return (flags & flag) != 0;
}

static void schedule_path(struct schedule_status *status, struct queue *queue,
			  const char *path, unsigned int flags);

static void schedule_directory(struct schedule_status *status, struct queue *queue,
			      const char *path, unsigned int flags)
{
	unsigned int len;
	char *child;
	DIR *dir;

	if (has_flag(flags, CRC32SUM_RECURSIVE) == false) {
		set_error(status, path, EISDIR);
		return;
	}

	dir = opendir(path);
	if (dir == NULL) {
		set_error(status, path, errno);
		return;
	}

	while (1) {
		struct dirent *de = readdir(dir);
		if (de == NULL)
			break;
		if (is_dot_or_dotdot(de->d_name))
			continue;

		len = strlen(path) + strlen(de->d_name) + 3;
		child = malloc(len);
		snprintf(child, len, "%s/%s", path, de->d_name);

		schedule_path(status, queue, child, flags);
		free(child);

		if (status->error) {
			break;
		}
	}

	closedir(dir);
}

static inline int is_loop(const char *path, const char *target)
{
	unsigned int i = 0;

	if (!path || !target)
		return false;

	if (strcmp(target, "/") == 0)
		return true;

	while (1) {
		if (path[i] == '/' && target[i] == '\0')
			return true;
		if (path[i] != target[i])
			return false;
		if (path[i] == '\0' || target[i] == '\0')
			break;
		i++;
	}

	return false;
}

static int check_link(const char *path, struct schedule_status *status)
{
	char *target;
	int ret = 0;

	target = realpath(path, NULL);
	if (target == NULL) {
		set_error(status, path, ENOLINK);
		return 1;
	}

	if (is_loop(path, target)) {
		set_error(status, path, ELOOP);
		ret = 2;
	}

	free(target);

	return ret;
}

static void schedule_symlink(struct schedule_status *status, struct queue *queue,
			     const char *path, unsigned int flags)
{
	struct stat sb;

	if (check_link(path, status) != 0) {
		/* error is already set */
		return;
	}

	if (stat(path, &sb) != 0) {
		set_error(status, path, errno);
		return;
	}

	if (S_ISDIR(sb.st_mode) && has_flag(flags, CRC32SUM_FOLLOW)) {
		schedule_directory(status, queue, path, flags);
		return;
	}

	schedule_file(queue, path, sb.st_size, 0);
}

static void schedule_path(struct schedule_status *status, struct queue *queue,
			  const char *path, unsigned int flags)
{
	struct stat sb;

	if (status->error)
		return;

	if (lstat(path, &sb) != 0) {
		set_error(status, path, errno);
		return;
	}

	if (S_ISDIR(sb.st_mode)) {
		schedule_directory(status, queue, path, flags);

	} else if (S_ISLNK(sb.st_mode)) {
		schedule_symlink(status, queue, path, flags);

	} else if (S_ISREG(sb.st_mode)) {
		schedule_file(queue, path, sb.st_size, 0);
	}
}

int queue_schedule_path(struct queue *queue, const char *path,
			unsigned int flags)
{
	struct schedule_status status = {
		.path = NULL,
		.error = 0
	};

	schedule_path(&status, queue, path, flags);
	if (status.error) {
		errno_to_error(status.error, status.path);
		free(status.path);
	}

	return -status.error;
}
