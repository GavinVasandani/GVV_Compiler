#ind_test.sh runs test script for an individual file:

rm test_program.s

make bin/c_compiler

bin/c_compiler -S test_program.c -o test_program.s

make clean