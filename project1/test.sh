#!/bin/sh
# This is a comment

rm *.txt
make clean
make

./common files/THEGODFATHER.txt files/THEGODFATHER2.txt student_out.txt
if diff -w student_out.txt files/correct_out.txt; then
    echo Test: Success-----------------------------------------------Success
else
    echo Test: Fail--------------------------------------------------Fail
fi
