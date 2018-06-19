#!/bin/sh
set -e

binary=crc32sum

test -f $binary || make

rm -rf ./test && mkdir ./test

echo -n "hello" >./test/hello
echo -n "world" >./test/world
dd if=/dev/zero of=./test/zeroes bs=1024 count=7 2>/dev/null
dd if=/dev/zero of=./test/big bs=8192 count=8192 2>/dev/null

./$binary ./test/hello ./test/world ./test/zeroes ./test/big >check

content=$(cat check)
if ! test x"$content" = x"\
3610a686  ./test/hello
3a771143  ./test/world
309137d4  ./test/zeroes
b2eb30ed  ./test/big"; then
	echo "\033[31merror: checksum generation failed.\033[0m"
	exit 1
fi

output=$(./$binary -c check)
if ! test x"$output" = x"\
./test/hello: OK
./test/world: OK
./test/zeroes: OK
./test/big: OK"; then
	echo "\033[31merror: checksum verification failed.\033[0m"
	exit 1
fi

output=$(./$binary -r test)

# The files may not be ordered.
if test -z "$(echo "$output" | grep 'b2eb30ed  test/big' | cat)" ||	\
   test -z "$(echo "$output" | grep '3610a686  test/hello' | cat)" ||	\
   test -z "$(echo "$output" | grep '3a771143  test/world' | cat)" ||	\
   test -z "$(echo "$output" | grep '309137d4  test/zeroes' | cat)"
then
	echo "\033[31merror: argument '--recursive' not working.\033[0m"
	exit 1
fi

output=$(./$binary -c check -q)
if ! test -z "$output"; then
	echo "\033[31merror: argument '--quiet' not working.\033[0m"
	exit 1
fi

sum=$(echo -n "hello" | ./$binary)
if ! test x"$sum" = x"3610a686"; then
	echo "\033[31merror: STDIN checksum generation failed.\033[0m"
	exit 1
fi

output=$(./$binary test 2>&1 | cat)
if ! test x"$output" = x"crc32sum: 'test' is a directory."
then
	echo "\033[31merror: directory sum test failed.\033[0m"
	exit 1
fi

chmod 0000 test/hello
output=$(./$binary test/hello 2>&1 | cat)
if ! test x"$output" = x"crc32sum: do not have access to 'test/hello'."
then
	echo "\033[31merror: inaccessible file sum test failed.\033[0m"
	exit 1
fi

rm -f ./check
rm -rf ./test

echo "\033[32msuccess\033[0m"
