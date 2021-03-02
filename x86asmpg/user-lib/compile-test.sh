#!/bin/bash
# Before running compile.sh: please move ld.script compile-test.sh outputlib.c
# checkfunctions.h to the same directory.

function compile_asm() {
    echo "compile $1"
    nasm -f elf $1 -o ${1%.s}.o
    return $?
}

function link_with_outputlib() {
    echo "link with outputlib"
    gcc -m32 -c -fno-builtin outputlib.c -o outputlib.o
    gcc -static -m32 -o ${1%.o} $1 outputlib.o -Wl,-Tld.script
    return $?
}

function gen_standard_results() {
    echo "print standard results into stdoutput.rst"
    ./$1 > stdoutput.rst
    return $?
}

function link_with_checklib() {
    echo "link with checklib"
    gcc -m32 -c -fno-builtin checklib.c -o checklib.o
    gcc -static -m32 -o $2 $1 checklib.o -Wl,-Tld.script
}

function print_help() {
    echo "Usage: bash compile-test.sh [file1] -o [file2]"
    echo "      --help/-h         show this text"
    echo "      file1              test assembly file"
    echo "      -o file2           output file"
}

case $# in
    1)
        if [ $1 == "--help" -o $1 == "-h" ]
        then
            print_help
            exit 0
        fi
        ;;
    3)
        if [ $2 == "-o" ]
        then
            compile_asm $1
            link_with_outputlib ${1%.s}.o
            if gen_standard_results ${1%.s}; then
                link_with_checklib ${1%.s}.o $3
            fi
            rm *.o
            exit 0
        fi
        ;;
    *)
        ;;
esac
echo "Wrong arguments!"
print_help
