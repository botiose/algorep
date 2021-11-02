#!/bin/bash

mode="release"
if [ $# -eq 1 ]; then
    mode=$1
fi

pwd="$(echo $PWD | sed -n 's/.*\/\(.*\)/\1/p')"
if [[ $pwd == "build" ]]; then
    cd ..
fi

mkdir -p build

cd build

arch=$(lscpu | sed -n 's/Architecture:[[:space:]]*\(.*\)/\1/p' | tr -d '\n')

if [ -d "$arch" ]; then
    echo "Removing existing $arch directory..."
    rm -rf "$arch"
fi

mkdir "$arch"

cd "$arch"

if [[ $mode == "debug" ]]; then
    cmake -D CMAKE_CXX_COMPILER=mpic++ -DCMAKE_BUILD_TYPE="Debug" ../..
elif [[ $mode == "release" ]]; then
    cmake -D CMAKE_CXX_COMPILER=mpic++ -DCMAKE_BUILD_TYPE="Release" ../..
else
    echo "Error: Valid options are 'release' or 'debug'"
    cd ../..
    exit 1
fi

cd ../..

config="$(cat etc/default.json | sed "s/ARCHITECTURE/$arch/")"

echo "$config" | sed "s|PROJECT_PATH|$(pwd)|" > etc/config.json
