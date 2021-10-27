#!/bin/bash

> "$1"

for i in $(seq 3); do
    line=$(echo "$RANDOM" | md5sum | head -c 10)
    echo "$line" >> "$1"
done

