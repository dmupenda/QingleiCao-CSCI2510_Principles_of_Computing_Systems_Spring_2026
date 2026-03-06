#!/bin/bash

cmd=$(wc -l "$1" | awk '{print $1}')
echo "LINES: $cmd"

echo "STATUS_COUNTS:"
awk '{print $2}' "$1" |sort| uniq -c| sort -nr|awk '{print $2, 41}'

