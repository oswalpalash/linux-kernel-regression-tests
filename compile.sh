#!/bin/bash
mkdir -p compiled
for f in crepros/*.c; do
	out="compiled/`basename "$f" .c`"
	if test -f "$out"; then
		continue
	fi
	echo $f
	flags=""
	if grep "__NR_mmap2" $f; then
		flags="-m32"
	fi
	gcc "$f" -static -pthread $flags -o $out
done
