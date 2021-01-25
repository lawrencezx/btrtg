#!/bin/bash
# Before running compile.sh: please move ld.script compile.sh outputlib.c
# checkfunctions.h test_intg.s to the same directory.

echo "compile test_intg.s"
nasm -f elf test_intg.s -o test_intg.o

echo "link with outputlib"
gcc -m32 -c -fno-builtin outputlib.c -o outputlib.o
gcc -static -m32 -o test_intg test_intg.o outputlib.o -Wl,-Tld.script

echo "print standard results into stdoutput.rst"
./test_intg > stdoutput.rst

if [ $? -eq 0 ]; then
  echo "link with checklib"
  gcc -m32 -c -fno-builtin checklib.c -o checklib.o
  gcc -static -m32 -o test_intg test_intg.o checklib.o -Wl,-Tld.script
fi

rm *.o
