/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __ERROR_H
#define __ERROR_H

void error(const char *err, ...);
void errno_to_error(int err, const char *path);

void set_error_handler(void (*handler)(const char *));

#endif /* __ERROR_H */
