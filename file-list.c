/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "file-list.h"

static struct file *file_alloc(const char *path, enum file_type type)
{
	struct file *file = malloc(sizeof(*file));

	if (file == NULL)
		return NULL;

	file->path = strdup(path);
	file->type = type;
	file->next = NULL;

	return file;
}
static void file_drop(struct file *file)
{
	if (file == NULL)
		return;

	free(file->path);
	free(file);
}

static int find_files(const char *path, struct file ***pp)
{
	struct stat sb;
	struct dirent *de;
	DIR *dir;
	char *child;
	unsigned int len;
	int retval = 0;

	if (stat(path, &sb) != 0)
		return -errno;

	if ((sb.st_mode & S_IFMT) == S_IFDIR) {
		dir = opendir(path);
		if (dir == NULL)
			return -errno;

		while ((de = readdir(dir)) != NULL) {

			if (is_dot(de->d_name)) {
				**pp = file_alloc(path, FILE_TYPE_DIR);
				*pp = &(**pp)->next;
				continue;
			} else if (is_dotdot(de->d_name)) {
				continue;
			}

			len = strlen(path) + strlen(de->d_name) + 3;
			child = malloc(len);
			snprintf(child, len, "%s/%s", path, de->d_name);

			if (de->d_type == DT_DIR) {
				if ((retval = find_files(child, pp)) != 0) {
					free(child);
					goto close_dir;
				}
			} else {
				**pp = file_alloc(child, FILE_TYPE_FILE);
				*pp = &(**pp)->next;
			}

			free(child);

		}
close_dir:
		closedir(dir);
	} else {
		**pp = file_alloc(path, FILE_TYPE_FILE);
		*pp = &(**pp)->next;
	}

	return retval;
}

struct file *file_list_create(const char *path)
{
	struct file *head = NULL, **pp = &head;

	if (find_files(path, &pp) != 0) {
		file_list_drop(head);
		return NULL;
	}

	return head;
}

void file_list_drop(struct file *head)
{
	struct file *walk = head, *next;

	while (walk) {
		next = walk->next;
		file_drop(walk);
		walk = next;
	}
}
