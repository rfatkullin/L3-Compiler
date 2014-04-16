#!/bin/sh

#Run from build directory

#Build
cmake -DCMAKE_BUILD_TYPE=Debug ..
flex ../frontend/lexer.l
bison -d --verbose ../frontend/parser.y
make

#Test
../tester.sh ./L3-Compiler ../tests
