#!/bin/sh
set -e

bin=crc32sum

test -f $bin || make
cd tests

# ** Computation ***************************************************************

# Single files checksums
output="$(./../$bin a b)"
test "x$output" = "x$(printf '5ecfe3c5  a\n6f27f958  b')" || {
	echo >&2 "fatal: single file checksum computation failed."
	exit 1
}

# Recursive checksums
output="$(./../$bin -r dir | sort)"
test "x$output" = "x$(printf '0cf7cc62  dir/d\nc950f2ec  dir/c')" || {
	echo >&2 "fatal: recursive checksum computation failed."
	exit 1
}

output="$(echo -n hello | ./../$bin)"
test "x$output" = "x3610a686" || {
	echo >&2 "fatal: STDIN checksum generation failed."
	exit 1
}

# ** Errors ********************************************************************

output="$(./../$bin dir 2>&1 || true)"
test "x$output" = "xcrc32sum: 'dir' is a directory." || {
	echo >&2 "fatal: crc of a dir should not be computed."
	exit 1
}

# ** Checking ******************************************************************

# Quiet verification
output="$(./../$bin -q -c badsums.txt || true)"
test "x$output" = "x./a: FAILED" || {
	echo >&2 "fatal: test of '-q' argument failed."
	exit 1
}

# Silent verification
output="$(./../$bin -s -c badsums.txt || true)"
test -z "$output" || {
	echo >&2 "fatal: test of '-s' argument failed."
	exit 1
}

# Normal usage - the sum file is in the working directory:
./../$bin -c sums.crlf.txt >/dev/null || {
	echo >&2 "fatal: sums.crlf.txt - verification failed."
	exit 1
}

./../$bin -c sums.lf.txt >/dev/null || {
	echo >&2 "fatal: sums.lf.txt - verification failed."
	exit 1
}

# Relative path to the sum file:
cd ..
./$bin -c tests/sums.lf.txt >/dev/null || {
	echo >&2 "fatal: relative path verification failed."
	exit 1
}

# ******************************************************************************

printf "\033[32msuccess\033[0m\n"
