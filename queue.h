/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __QUEUE_H
#define __QUEUE_H

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
	struct file **tail; /* internal */
};

void queue_init(struct queue *queue);
void queue_clear(struct queue *queue);

int queue_schedule_regular_file(struct queue *queue, const char *path,
				void *userdata);
int queue_schedule_path(struct queue *queue, const char *path,
			unsigned int flags);

#endif /* __QUEUE_H */
