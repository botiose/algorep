#!/bin/bash

config="$(cat ../../etc/config.json)"

while getopts "v:" o; do
    case "${o}" in
        v)
            v="valgrind ${OPTARG}"
            ;;
        *)
            echo "Usage: launch-client.sh [-v <flags>]"
            exit 1
            ;;
    esac
done

clientId=0

for archs in $(jq -rc ".nodes | to_entries[]" <<< "$config"); do
    arch=$(jq -rc ".key" <<< "$archs")
    for node in $(jq -rc ".value[]" <<< "$archs"); do
        coreCount=$(jq -rc ".clientProcessCount" <<< "$node")

        if [ $coreCount -eq 0 ]; then
            continue
        fi
        
        address=$(jq -rc ".address" <<< "$node")
        projectPath=$(jq -rc ".projectPath" <<< "$node")

        for i in $(seq $coreCount); do
            clientDir="$(printf "%02d\n" $clientId)"

            mkdir ../../etc/client/"$clientDir"
            touch ../../etc/client/"$clientDir"/repl.txt
            ./../../etc/script/gen-commands.sh ../../etc/client/"$clientDir"/command.txt
            
            if [ "$address" == "localhost" ]; then
                mpirun --ompi-server file:../../etc/urifile --host localhost $v "$projectPath/build/$arch/bin/client" "$clientDir" "$coreCount" &
            else
                ssh "$address" "cd $projectPath && mpirun --ompi-server file:etc/urifile --host localhost $v "$projectPath/build/$arch/bin/client" "$clientDir" "$coreCount""
            fi
            
            clientId=$((clientId+1))
        done
    done
done
