BTRTG
===========================

BTRTG (Binary Translation Random Test Generator) is a x86 assembly program generator. The generated programs are used to test binary translator.

This project is modified from [NASM](https://github.com/netwide-assembler/nasm) (the Netwide Assembler).

## build

```shell
mkdir build
cmake .. && make
cmake .. && make
```

## usage

**How to generate a test by a template?**

Run the commands below and a new file *add.s* is generated.

```shell
(build)$ ./btrtg/btrtg --help
Usage: btrtg -k <path> -g <path> -t <file> [options]
Options:
    --help        show this text and exit (also -h)
    -h            show this text and exit (also --help)
    -k <path>     path to the testing knowledge template files
    -g <path>     path to the instruction group template files
    -t <file>     select a template <file>
    -o <file>     place the output into <file>
(build)$ ./btrtg/btrtg -t ../xmlmodel/templates/unit/General-Purpose/binary-arithmetic/add.xml -o add.s
```

**How to compile a generated testing file?**

Move the file into *user-lib* directory and use script *compile-test.sh* to compile. Run the commands below and the generated executable file add.test is the final testing file.

```
(build)$ cd user-lib
(user-lib)$ cp ../add.s ./
(user-lib)$ bash compile-test.sh add.s -o add.test
```

**How to generate/compile all the tests by lots of templates in a time?**

Use the script *run-all-templates.sh*. Exp, command

 ```shell
bash run-all-templates.sh ../../xmlmodel/templates/unit/MMX -k ../../xmlmodel/tks -g ../../xmlmodel/templates/group -o results/unit/MMX -generate 
 ```

will generate all the testings by templates in *xmlmodel/templates/unit/MMX* directory.

**How to write a template?**

Follow the rules in template writing [specification](./docs/模板书写规范.md). Also you can refer to the existed templates.

