/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __FILE_LIST_H
#define __FILE_LIST_H

struct file {
	char *path;
	unsigned long size;
	void *userdata;
	struct file *next;
};

struct queue {
	unsigned int files;
	unsigned long nbytes;
	struct file *head;
	struct file **tail;
};

static inline int is_dot_or_dotdot(const char *name)
{
	return name[0] == '.' && (name[1] == '\0' ||
		(name[1] == '.' && name[2] == '\0'));
}

void queue_init(struct queue *queue);
void queue_clear(struct queue *queue);

int queue_schedule_regular_file(struct queue *queue, const char *path,
				void *userdata);
int queue_schedule_path(struct queue *queue, const char *path,
			unsigned int flags);

#endif /* __FILE_LIST_H */
