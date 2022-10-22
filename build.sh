#!/bin/bash

cd ./run_tree

echo Building C Tests
clang ../test/test_basic.c -g -O0     -o c_test_basic -fdiagnostics-absolute-paths
# ./c_test_basic --filter=sort.radix_u32

echo Building CPP Tests
# clang++ ../test/test_basic.cpp -o cpp_test_basic

cd ..