#include "compat.h"
#include "error.h"

#define PROGNAME PACKAGE_NAME

static void print_error_default(const char *message)
{
	fprintf(stderr, "%s\n", message);
}

void (*print_error)(const char *) = print_error_default;

void set_error_handler(void (*handler)(const char *))
{
	print_error = handler;
}

#undef error
void error(const char *err, ...)
{
	char buf[1024] = PROGNAME ": ";
	va_list params;

	va_start(params, err);
	vsprintf(buf + strlen(buf), err, params);

	print_error(buf);

	va_end(params);
}

void errno_to_error(int err, const char *path)
{
	switch (err) {
	case ENOENT:
		error("'%s' not found.", path);
		break;
	case EACCES:
		error("do not have access to '%s'.");
		break;
	case EINVAL:
		error("'%s' is not regular file.", path);
		break;
	case ELOOP:
		error("'%s' a link loop detected.", path);
		break;
	case ENOLINK:
		error("'%s' is a broken link.", path);
		break;
	case EISDIR:
		error("'%s' is a directory.", path);
		break;
	default:
		error("cannot open '%s': error %d.", path, err);
	}
}
