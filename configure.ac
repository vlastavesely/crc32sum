#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([crc32sum], [1.1.0], [vlastavesely@protonmail.ch])
AC_CONFIG_SRCDIR([crc32.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strdup strtol])

# Enable SIMD optimisations
USE_SIMD=no
AC_MSG_CHECKING([whether SIMD optimisations should be enabled])
AC_ARG_ENABLE(simd,
	AS_HELP_STRING([--enable-simd], [enable SIMD optimisations]), [
		AC_MSG_RESULT(yes)
		USE_SIMD=yes
], [AC_MSG_RESULT(no)])

if test "x$USE_SIMD" = xyes; then
	AX_GCC_X86_CPU_SUPPORTS([sse4.1])
fi

AC_SUBST(USE_SIMD)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
