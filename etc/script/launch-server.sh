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

agrs=""
for archs in $(jq -rc ".nodes | to_entries[]" <<< "$config"); do
    arch=$(jq -rc ".key" <<< "$archs")
    for node in $(jq -rc ".value[]" <<< "$archs"); do
        coreCount=$(jq -rc ".serverProcessCount" <<< "$node")

        if [ $coreCount -eq 0 ]; then
            continue
        fi
        
        address=$(jq -rc ".address" <<< "$node")
        host=$(cut -d'@' -f2 <<< "$address")
        projectPath=$(jq -rc ".projectPath" <<< "$node")
        args="$args --host $host:$coreCount $v $projectPath/build/$arch/bin/server :"
    done
done

mpirun --ompi-server file:../../etc/urifile ${args::-1}
