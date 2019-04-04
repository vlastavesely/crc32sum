/* SPDX-License-Identifier: GPL-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "progress.h"

static struct winsize w;
static int initialized = 0;

static void progress_set_window_height(int height)
{
	fprintf(stderr, /* save cursor */
			"\n\0337"
			/* set scroll region (places cursor to the top left) */
			"\033[0;%dr"
			/* restore cursor but ensure it is inside the scrolling area */
			"\0338\033[1A\033[J",
			height);
	fflush(stderr);
}

static void progress_init()
{
	ioctl(STDERR_FILENO, TIOCGWINSZ, &w);
	progress_set_window_height(w.ws_row - 1);
}

static void progress_abort()
{
	progress_stop();
	exit(0);
}

void progress_add(struct progress *ctx, unsigned long val)
{
	ctx->pos += val;
	progress_step((float) ctx->pos / (float) ctx->max);
}

void progress_start()
{
	if (isatty(STDERR_FILENO) == 0) {
		errno = 0;
		return;
	}

	signal(SIGWINCH, progress_init);
	signal(SIGINT, progress_abort);
	signal(SIGKILL, progress_abort);
	progress_init();
	initialized = 1;
	progress_step(0);
}

void progress_step(float val)
{
	char *bar;
	int max;

	if (isatty(STDERR_FILENO) == 0) {
		errno = 0;
		return;
	}

	if (initialized == 0)
		progress_start();

	bar = malloc(w.ws_col);

	max = (w.ws_col - 20);
	memset(bar, '.', max);
	memset(bar, '#', (int) (max * val));
	bar[max] = 0;

	fprintf(stderr, "\e[s\e[%d;0H\e[42;30mProgress: [%3d%%]\e[0m [%s]\e[u",
			w.ws_row + 1, (int) (val * 100.0), bar);
	fflush(stderr);
	free(bar);
}

void progress_stop()
{
	if (isatty(STDERR_FILENO) == 0) {
		errno = 0;
		return;
	}

	progress_set_window_height(w.ws_row);
}
