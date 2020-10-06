#!/usr/bin/env bash

limits=(10 100 200 500 1000 1500)
loop=1000
for limit in "${limits[@]}"; do
    hyperfine \
	-i -s basic \
	"php select-mysql.php $limit $loop" \
	"php select-orzclick.php $limit $loop" \
	"php select-seasclick.php $limit $loop" | tee -a result-select.txt
done
