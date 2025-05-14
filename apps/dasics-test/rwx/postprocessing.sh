#!/bin/bash

if [ -f "syscall.json" ]; then
    echo "syscall.json file exists, proceeding with post-processing."
    python ../utils/patchelf.py -i syscall.json -e build/rwx

else
    echo "syscall.json file does not exist, exiting."
    exit 1
fi