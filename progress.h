/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __PROGRESS_H
#define __PROGRESS_H

struct progress {
	unsigned long max;
	unsigned long pos;
	void (*add)(struct progress *ctx, unsigned long val);
};

void progress_add(struct progress *ctx, unsigned long val);

void progress_start(void);
void progress_step(float progress);
void progress_stop(void);

#endif /* __PROGRESS_H */
