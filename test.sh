#!/bin/sh
set -e

test -f crc32 || make

echo -n "hello" >hello
echo -n "world" >world
dd if=/dev/zero of=zeroes bs=1024 count=7 2>/dev/null
dd if=/dev/zero of=big bs=8192 count=8192 2>/dev/null

./crc32 hello world zeroes big >check

content=$(cat check)
if ! test x"$content" = x"3610a686  hello
3a771143  world
309137d4  zeroes
b2eb30ed  big"; then
	echo "\033[31merror: checksum generation failed.\033[0m"
	exit 1
fi

output=$(./crc32 -c check)
if ! test x"$output" = x"hello: OK
world: OK
zeroes: OK
big: OK"; then
	echo "\033[31merror: checksum verification failed.\033[0m"
	exit 1
fi

sum=$(echo -n "hello" | ./crc32)
if ! test x"$sum" = x"3610a686"; then
	echo "\033[31merror: STDIN checksum generation failed.\033[0m"
	exit 1
fi


rm hello world zeroes big check

echo "\033[32msuccess\033[0m"
