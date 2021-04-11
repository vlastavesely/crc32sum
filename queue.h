/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __QUEUE_H
#define __QUEUE_H

struct file {
	char *path;
	unsigned long size;
	unsigned int sum;
};

struct queue {
	struct file *files;
	unsigned int nfiles;
	unsigned long nbytes;
};

void queue_init(struct queue *queue);
void queue_clear(struct queue *queue);

int queue_schedule_regular_file(struct queue *queue, const char *path,
				unsigned int sum);
int queue_schedule_path(struct queue *queue, const char *path,
			unsigned int flags);

#endif /* __QUEUE_H */
