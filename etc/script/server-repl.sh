#!/bin/bash

isUp=0

while [ $isUp -eq 0 ]; do
    read line

    command=$(cut -d',' -f2 <<< "$line")

    echo "$line" >> etc/server/repl.txt
    
    $(test "$line" != "shutdown")
    isUp=$?
done

mpirun --host localhost /home/otiose/repos/epita/algorep/build/x86_64/bin/shutdown-client
