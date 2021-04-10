#ifndef __COMPAT_H
#define __COMPAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <errno.h>

#define HAVE_SSE41 1 /* FIXME */

#ifndef ALIGN
#define ALIGN(n) __attribute__((aligned(n)))
#endif

#endif /* __COMPAT_H */
