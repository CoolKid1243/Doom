#!/bin/bash
set -e

mkdir -p build
cd build
cmake -S .. -B .
make -j$(nproc)
./DOOM
cd ..