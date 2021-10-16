#!/bin/bash

config="$(cat etc/config.json)"

if [ ! ${PWD##*/} == "algorep" ]; then
    echo "ssh-mount.sh: Should should be executed from the project's root directory."
    exit 1
fi

for archs in $(jq -rc ".nodes | to_entries[]" <<< "$config"); do
    for node in $(jq -rc ".value[]" <<< "$archs"); do
        address=$(jq -rc ".address" <<< "$node")

        if [ "$address" == "localhost" ]; then
            continue
        fi

        projectPath=$(jq -rc ".projectPath" <<< "$node")

        if [ "$1" == "-u" ]; then
            ssh "$address" "fusermount -u $projectPath"
            echo "Unmounted: $address"
        elif [ -z "$1" ]; then
            ssh "$address" "sshfs $(whoami)@$(hostname):$(pwd) $projectPath"
            echo "Mounted: $address"
        else
            echo "ssh-mount.sh: Usage: ssh-mount.sh [-u]"
            exit 1
        fi
    done
done
