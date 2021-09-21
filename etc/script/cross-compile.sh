#!/bin/bash

if [ ! $# -eq 1 ]; then
    echo "Usage: cross-compile.sh <target>"
    exit 1
fi

config="$(cat ../../etc/config.json)"

for archs in $(jq -rc ".nodes | to_entries[]" <<< "$config"); do
    for archGroup in $archs; do
        address=$(jq -rc ".value[0].address" <<< "$archGroup")

        if [ "$address" == "localhost" ]; then
            break
        fi

        arch=$(jq -rc ".key" <<< "$archGroup")
        projectPath=$(jq -rc ".value[0].projectPath" <<< "$archGroup")
        
        ssh "$address" "cd $projectPath/build/$arch && make $1"
    done
done
