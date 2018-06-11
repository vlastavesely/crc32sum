/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __FILE_LIST_H
#define __FILE_LIST_H

enum file_type {
	FILE_TYPE_FILE = 0,
	FILE_TYPE_DIR = 1
};

struct file {
	char *path;
	enum file_type type;
	struct file *next;
};

static inline int is_dot(const char *name)
{
	return (name[0] == '.' && name[1] == '\0');
}

static inline int is_dotdot(const char *name)
{
	return (name[0] == '.' && name[1] == '.' && name[2] == '\0');
}

struct file *file_list_create(const char *path);
void file_list_drop(struct file *head);

#endif /* __FILE_LIST_H */
