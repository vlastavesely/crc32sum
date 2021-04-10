/* SPDX-License-Identifier: GPL-2.0 */

#include "compat.h"
#include "progress.h"

/*
 * The code is based on following sources:
 *
 * https://salsa.debian.org/apt-team/apt/blob/master/apt-pkg/install-progress.cc
 * https://salsa.debian.org/apt-team/apt/blob/master/apt-pkg/install-progress.h
 */

static struct winsize w;
static int initialized = 0;

static void progress_start();
static void progress_stop();

/* https://salsa.debian.org/apt-team/apt/blob/master/apt-pkg/install-progress.cc#L271-282 */
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

static void progress_step(float val)
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

static void progress_add(struct progress *ctx, unsigned long val)
{
	ctx->pos += val;
	progress_step((float) ctx->pos / (float) ctx->max);
}

static void progress_start()
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

static void progress_stop()
{
	if (isatty(STDERR_FILENO) == 0) {
		errno = 0;
		return;
	}

	progress_set_window_height(w.ws_row);
}

struct progress *progress_alloc(unsigned long max)
{
	struct progress *progress;

	progress = malloc(sizeof(*progress));
	if (progress == NULL)
		return NULL;

	progress->max = max;
	progress->pos = 0;
	progress->add = progress_add;

	return progress;
}

void progress_drop(struct progress *progress)
{
	progress_stop(progress);
	free(progress);
}
