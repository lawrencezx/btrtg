#!/bin/bash

function print_help() {
    echo "Usage: bash run-all-templates.sh <path> -k <path> -g <path> -o <path> [options]"
    echo "Options:"
    echo "      --help/-h           show this text"
    echo "      <path>              path to template files"
    echo "      -k <path>           path to the testing knowledge template files"
    echo "      -g <path>           path to the instruction group template files"
    echo "      -o <path>           path to the output template files"
    echo "      -generate           only generate tests"
    echo "      -compile            only compile tests"
}

function check_tools() {
    which btrtg
    if [ `witch` == 1 ]; then
        echo "Error: please install btrtg!"
        exit 1
    fi
}

function generate_all_tmplt() {
    check_tools
    for file in `cd $1 && find ./ -name "*.xml"`
    do
        path=$4/${file%/*}
        testFile=$4/${file%.xml}.test.s
        mkdir -p $path
        echo -e "\033[31mbtrtg -k $2 -g $3 -t $1/$file -o $testFile \033[0m"
        btrtg -k $2 -g $3 -t $1/$file -o $testFile
    done
}

function compile_all_tmplt() {
    for file in `cd $1 && find ./ -name "*.xml"`
    do
        testFile=$4/${file%.xml}.test.s
        echo -e "\033[31mbash compile-test.sh $testFile -o ${testFile%.s} \033[0m"
        bash compile-test.sh $testFile -o ${testFile%.s}
    done
}

groupPath=""
tkPath=""
outputPath=""
tmpltPath=""
generate=0
compile=0

arr=($@)

if [ ${arr[0]} == "--help" -o ${arr[0]} == "-h" ]; then
    print_help
    exit 0
fi

for(( i=0;i<${#arr[@]};i++))
do
    arg=${arr[i]}
    if [ $arg == "-g" ]; then
        groupPath=${arr[i+1]}
        let i++
    elif [ $arg == "-k" ]; then
        tkPath=${arr[i+1]}
        let i++
    elif [ $arg == "-o" ]; then
        outputPath=${arr[i+1]}
        let i++
    elif [ $arg == "-generate" ]; then
        generate=1
    elif [ $arg == "-compile" ]; then
        compile=1
    else
        tmpltPath=$arg
    fi
done

if [ groupPath == "" ]; then
    echo "Error: no instruction group templates path found!"
    print_help
    exit 0
fi
if [ tkPath == "" ]; then
    echo "Error: no testing knowledge templates path found!"
    print_help
    exit 0
fi
if [ outputPath == "" ]; then
    echo "Error: no output path found!"
    print_help
    exit 0
fi

if [ $generate == 1 ]; then
    generate_all_tmplt $tmpltPath $tkPath $groupPath $outputPath
elif [ $compile == 1 ]; then
    compile_all_tmplt $tmpltPath $tkPath $groupPath $outputPath
else
    generate_all_tmplt $tmpltPath $tkPath $groupPath $outputPath
    compile_all_tmplt $tmpltPath $tkPath $groupPath $outputPath
fi 
