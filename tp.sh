cd src
bison -d -t test.y
flex c_lexer.fl
g++ -o out test.tab.c lex.yy.c
./out ../compiler_tests/_example/example.c
