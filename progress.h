/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __PROGRESS_H
#define __PROGRESS_H

struct progress {
	unsigned long max;
	unsigned long pos;
	void (*add)(struct progress *ctx, unsigned long val);
};

struct progress *progress_alloc(unsigned long max);
void progress_drop(struct progress *progress);

#endif /* __PROGRESS_H */
