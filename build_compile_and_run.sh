#!/bin/sh

#Run from build directory

cmake ..
flex ../frontend/lexer.l
bison -d ../frontend/parser.y
make
../compile_and_run.sh