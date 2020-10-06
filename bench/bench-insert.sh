#!/usr/bin/env bash

# Notice:
# clickhouse-cpp has a default buffer of 8192 bytes, so we start at 275 to
# ensure the buffer is filled. This reduce the affect of Nagle's algorithm,
# and provides more consistent test results for SeasClick.

inserts=(275 500 1000 2000 5000 10000)
loop=500
for insert in "${inserts[@]}"; do
    hyperfine \
	-i -m 3 -M 10 -s basic \
	"php insert-orzclick.php $insert $loop" \
	"php insert-orzclick-columnar.php $insert $loop" \
	"php insert-orzclick-indexed.php $insert $loop" \
	"php insert-seasclick-block.php $insert $loop" \
	"php insert-seasclick.php $insert $loop" | tee -a result-insert.txt
done
