#!/bin/bash

isUp=0

while [ $isUp -eq 0 ]; do
    read line

    command=$(cut -d',' -f2 <<< "$line")

    echo "$line" >> etc/server/repl.txt
    
    $(test "$line" != "shutdown")
    isUp=$?
done

mpirun --ompi-server file:etc/urifile --host localhost bin/shutdown-client > /dev/null
