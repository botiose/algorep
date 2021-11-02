#!/bin/bash

config="$(cat etc/config.json)"

serverCount=$(sed -n 's/.*serverProcessCount": \(.*\),.*/\1/p' <<< "$config")

mpirun --ompi-server file:../../etc/urifile --host "localhost:$serverCount" "bin/server"
