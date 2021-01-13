#!/bin/bash
# Before running compile.sh: please move ld.script compile.sh printlib.c
# test_intg.s to the same directory.

gcc -m32 -c -fno-builtin printlib.c -o printlib.o
nasm -f elf test_intg.s -o test_intg.o
gcc -static -m32 -o test_intg test_intg.o printlib.o -Wl,-Tld.script
