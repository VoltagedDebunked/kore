#!/bin/bash

# Change to the directory containing the source files
cd src || exit

# Find and format all .c and .h files
find . -name '*.c' -o -name '*.h' | xargs clang-format -i
