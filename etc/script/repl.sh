#!/bin/bash

isUp=0

if [ ! $# -eq 1 ]; then
    echo "Usage: repl.sh <filepath>"
    exit 1
fi

if [ ! -f $1 ]; then
    echo "repl.sh: file not found"
    exit 1
fi
    

while [ $isUp -eq 0 ]; do
    read line

    command=$(cut -d',' -f2 <<< "$line")

    echo "$line" >> "$1"
    
    $(test "$command" != "shutdown")
    isUp=$?
done

mpirun --ompi-server file:etc/urifile --host localhost bin/shutdown-client
