#!/bin/sh

#Run from build directory

echo "Run L3-Compiler..."
./L3-Compiler ../test_source.l3 test_source.il

res=$?
if [ $res != 0 ] ; then
    echo "L3-Compiler failed!"
    exit $res
fi

echo "Run ilasm..."
ilasm test_source.il > /dev/null

res=$?
if [ $res != 0 ] ; then
    echo "ilasm failed!"
    cat test_source.il
    exit $res
fi

echo "Run target program..."
./test_source.exe