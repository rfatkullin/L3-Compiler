#!/bin/sh

#Run from build directory

#Build
cmake ..
flex ../frontend/lexer.l
bison -d ../frontend/parser.y
make

#Test
../tester.sh ./L3-Compiler ../tests