#!/bin/bash

config="$(cat etc/config.json)"

clientCount=$(sed -n 's/.*clientProcessCount": \(.*\),.*/\1/p' <<< "$config")

for i in $(seq 0 $((clientCount-1))); do
    clientDir="$(printf "%02d\n" $i)"

    mkdir etc/client/"$clientDir"
    touch etc/client/"$clientDir"/repl.txt
    ./etc/script/gen-commands.sh etc/client/"$clientDir"/command.txt
    
    mpirun --ompi-server file:etc/urifile --host localhost $v "bin/client" "$clientDir" "$clientCount" &
done


