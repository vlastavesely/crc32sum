/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "queue.h"


static void queue_schedule_file(struct queue *queue, const char *path,
				unsigned int size)
{
	struct file *file;

	file = calloc(1, sizeof(*file));
	file->path = strdup(path);
	file->size = size;

	*queue->tail = file;
	queue->tail = &(*queue->tail)->next;
	queue->files++;
	queue->nbytes += size;
}

static void file_drop(struct file *file)
{
	if (file == NULL)
		return;

	free(file->path);
	free(file);
}

static void file_list_drop(struct file *head)
{
	struct file *walk = head, *next;

	while (walk) {
		next = walk->next;
		file_drop(walk);
		walk = next;
	}
}

void queue_init(struct queue *queue)
{
	queue->files = 0;
	queue->nbytes = 0;
	queue->head = NULL;
	queue->tail = &queue->head;
}

void queue_clear(struct queue *queue)
{
	file_list_drop(queue->head);
	queue_init(queue);
}

int queue_schedule_path(struct queue *queue, const char *path)
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

			retval = queue_schedule_path(queue, child);
			free(child);
			if (retval != 0)
				goto close_dir;
		}
close_dir:
		closedir(dir);
	} else {
		queue_schedule_file(queue, path, sb.st_size);
	}

	return retval;
}
