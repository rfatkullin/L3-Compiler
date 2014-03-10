#!/bin/bash

cd bin
flex ../frontend/lexer.l
bison -d ../frontend/parser.y
g++ -g -I. -I../frontend/ -I../backend -o compiler lex.yy.c parser.tab.c ../backend/compiler.h ../backend/compiler.cpp
./compiler ../test_source.l3 test_source.il
#ilasm test_source.il
#./il_source