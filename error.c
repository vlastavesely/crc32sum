#include "compat.h"
#include "error.h"

#define PROGNAME PACKAGE_NAME

#undef error
void error(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	fprintf(stderr, PROGNAME ": ");
	vfprintf(stderr, err, params);
	fputc('\n', stderr);
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
	case EISDIR:
		error("'%s' is a directory.", path);
		break;
	default:
		error("cannot open '%s': error %d.", path, err);
	}
}
