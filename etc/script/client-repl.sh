#!/bin/bash

isUp=0

while [ $isUp -eq 0 ]; do
    read line

    if [ "$line" == "start" ] || [ "$line" == "shutdown" ]; then
        for dir in $(ls etc/client); do
            echo "$line" >> "etc/client/$dir/repl.txt"
        done
    else
        rawClientId=$(cut -d',' -f1 <<< "$line")
        command=$(cut -d',' -f2 <<< "$line")
        clientId="$(printf "%02d\n" $rawClientId)"
        
        echo "0,$command" >> "etc/client/$clientId/repl.txt"
    fi
    
    $(test "$line" != "shutdown")
    isUp=$?
done

echo "shutdown" >> "etc/turn.txt"
