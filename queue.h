/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __FILE_LIST_H
#define __FILE_LIST_H

struct file {
	char *path;
	unsigned int size;
	struct file *next;
};

struct queue {
	unsigned int files;
	struct file *head;
	struct file **tail;
};

static inline int is_dot_or_dotdot(const char *name)
{
	return name[0] == '.' && (name[1] == '\0' ||
		(name[1] == '.' && name[2] == '\0'));
}

void queue_init(struct queue *queue);
int queue_schedule_path(struct queue *queue, const char *path);
void file_list_drop(struct file *head);

#endif /* __FILE_LIST_H */
