List of test files AST builds correctly:

bin/c_compiler -S compiler_tests/integer/testfile.c -o src/testadd.s

bin/c_compiler -S compiler_tests/custom/first_compile_test.c -o src/testadd.s

- compiler_tests/integer - all tests building
- compiler_tests/functions - all tests building
- compiler_tests/default - all tests building

Finished but need to test: function prototypes, function definitions, most simple types, string/char literals, for loop, if statement, nested for loops, nested if statements, multiple function declarations/definitions, n sized arguments in function definition. Need to add logical operators. Arrays, Else statement, While loop.

Started codegen, example code works but need to refine further to keep track of what variables are in the function for stack pointer stuff to work.

Things to do:
1. Add function to pass the stack offset to return statement
2. Add array assign and array create statements.
3. Add functions for search in stack for variable name.
4. Add data structure that stores all currently alive variables and information about whether its stored in stack or register.
5. Change codegen so it takes in address of variable from stack and stores it in an available register.
6. Do normal codegen then.

GTA Notes:

Filip answers:

- Store variable immediately as a place on the stack.
- Have symbol table which maps a variable to a variable_state object? Or just have a vector of variable_state objects that represent all the variables currently in scope.
- For each variable mention properties like: type, name, kind, isPntr, is in_stack_mem? If in stack memory what’s its exact offset from current sp (or fp?), in_reg, if so what register stored in.
- Store everything immediately, only use 3 regs. (Not have to but maybe).
- When calling variable immediately load into a temporary register or something.
- Put all arguments in stack, there will definitely be cases where number of arguments > 6 so put in stack and only call when its needed in function. So not all argument registers are needed.
- Symbol table for the variables in scope.
- Can also have data structure for registers and memory that manage these arenas and mirrors it so we can evaluate what’s used and allocate accordingly, but must update all every time we do something.
- Store on symbol table whether isPntr as this is important for pointer arithmetic as if we know isPntr then can do certain arithmetic jumps.
- When allocating function variables to stack, parse the AST once: find all variables in function and allocate memory for all in stack. Then parse again properly and evaluate constructs and stuff.
- When doing add, sub instruction, store the value in a register that’s free (so search for free register and store in that), instead of reassigning one of the operand registers as we’ll need that later on. Then maybe put a priority on the registers, so if a register stores a result it is low priority so it can be easily reassigned.
- Do array, array and pntr is similar.
- Do case statement. Do logical and with branching.

2022/2023 Compilers Coursework
==============================

There are two components to the coursework:

- [*A C compiler*](c_compiler.md), worth 90%. The source language is pre-processed C90, and the target language is RISC-V assembly. The target environment is Ubuntu 22.04, as described in the attached [Dockerfile](Dockerfile). See [here](./c_compiler.md) for the full set of requirements and more information about the testing environment.

- [*Evidence of time-tracking/project management*](management.md), worth 10%. This will be assessed orally at the start of Summer term. See [here](management.md) for more information about this component.

Repositories
============

Each group gets a bare private repository. It is up to you if you want to clone the main specification, or to start from scratch.

Submission
==========

The deadline for submitting your C compiler is **Friday 24 March 2023 at 23:59**. There is no deadline for the project management component; instead, this will be assessed by a short oral viva that will be organised in Summer term.

Submission will be via GitHub (code) and Teams (commit hash), as in the labs.

All submissions will be tested functionally -- there is no expectation for your compiler to *optimise* its input. Moreover, your compiler will only be tested on *valid* inputs, so you do not need to handle faulty inputs in a graceful way.

Changelog
=========

* New for 2022/2023:

    * Target architecture is now RISC-V rather than MIPS, in order to align with the modernised Instruction Architectures half of the module.
    * Instead of Vagrant, Docker is now used for the testing environment (with optional VS Code support).
    * Test scripts are now provided to check your compiler against the set of public tests, without having to write this yourself.
    * The basic compiler framework has been improved to support command line arguments.
    * GitHub Actions can now perform automated testing of your compiler.

* New for 2021/2022:

    * Various improvements to scripts for running test cases.

* New for 2020/2021:

    * In previous years, students were additionally required to submit a C-to-Python translator, as a "ramping up" task. This extra deliverable has been removed, as the labs provide plenty of "ramping up" practice.

    * We have provided a really basic compiler that simply ignores its input and produces a fixed, valid MIPS assembly program. This should help you to get started a bit more rapidly.

* New for 2019/2020:

    * In previous years, students were additionally required to submit a set of testcases. This deliverable has been removed; instead, a large collection of testcases has been provided for you, as this was judged to be more useful.

    * In previous years, the compiler component counted for 42.8% of the module; it now counts for 55%. It was felt that this weighting more accurately reflects the effort that students put in to building a working compiler.

Acknowledgements
================

* The coursework was originally designed by [David Thomas](https://www.southampton.ac.uk/people/5z9bmb/professor-david-thomas), who lectured this module until 2017-18. It is nowadays maintained by [John Wickerson](https://johnwickerson.github.io/), to whom any feedback should be sent.
* Thanks to [Yann Herklotz](https://yannherklotz.com/) for making various improvements to the compiler-testing scripts.
* Thanks to [Archie Crichton](https://www.doc.ic.ac.uk/~ac11018/) for providing a basic "getting started" compiler.
* Extra-special thanks to [James Nock](https://www.linkedin.com/in/jpnock) for overhauling the scripts for configuring the development environment, for writing detailed instructions for setting this up on various operating systems, and for creating GitHub actions capable of automatically testing compilers.
