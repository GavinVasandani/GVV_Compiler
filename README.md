**GVV Compiler:** *C to RISC-V Compiler*
==============================

![GVV Compiler](https://github.com/GavinVasandani/GVV_Compiler)

GVV Compiler is a C90 to RISC-V compiler developed as part of the EIE Instruction Set Architecture and Compilers coursework. The compiler supports arrays, recursion, floatting-point arithmetic and much more. The compiler also contains a stack simulator for RISC-V memory emulation. For debugging purposes, the compiler also contains an AST visualizer. The compiler has improved memory allocation with static memory analysis.

This compiler passed over 90% of given test cases. 

Building Compiler
--------
The compiler is built using the `make bin/c_compiler` command.

Run individual tests
--------
To run a C90 test program and view the generated assembly code (useful for debugging and code tracing): 

1. Copy the C90 test program into `test_program.c`. 

Run the command:

    ./ind_test.sh

2. The output assembly file can be viewed in `test_program.s`.

Run all tests and run on RISC-V emulator
--------
To test the compiler's correctness we:

1. Run all the C test programs through the compiler and generate the RISC-V assembly code 

2. Feed the RISC-V code through RISC-V emulator and compare the output with the solution set.

We can run all tests by running the command:

    ./test.sh

Repository Guide
--------

src
--------
The src directory includes the code for the parser, lexer and the generate AST code. The code contains classes for different types of C statements, operators, data structures and data types as well as the accompanying RISC-V assembly code conversion. The compiler is called by running the `compiler.cpp` file which contains main.

Dockerfile
--------
The dockerfile contains the setup for the docker container needed to run the compiler on. The dockerfile contains necessary runtime debuggers like Valgrind. The container setup also includes bison, flex dependencies needed for parsing and lexing.

Makefile
--------
The makefile contains the necessary build processes needed to build the complete compiler and feed the generated RISC-V assembly code into the RISC-V emulator to check for correctness.

compiler_tests
--------
Compiler tests contains all the C test cases. The test cases are categorized into control-flow, arithmetic, recursion tests and more.

