CPPFLAGS += -std=c++20 -W -Wall -g -I include

.PHONY: default

default: bin/c_compiler

src/c_parser_ast.tab.cpp src/c_parser_ast.tab.hpp  : src/c_parser_ast.y
	bison -v -d src/c_parser_ast.y -o src/c_parser_ast.tab.cpp

src/c_lexer_ast.yy.cpp : src/c_lexer_ast.fl src/c_parser_ast.tab.hpp
	flex -o src/c_lexer_ast.yy.cpp  src/c_lexer_ast.fl

bin/c_compiler : src/cli.cpp src/compiler.cpp src/c_parser_ast.tab.o src/c_lexer_ast.yy.o
	@mkdir -p bin
	g++ $(CPPFLAGS) -o bin/c_compiler $^

clean :
	rm -rf bin/*
	rm src/c_parser_ast.tab.cpp
	rm src/c_parser_ast.output
	rm src/c_parser_ast.tab.hpp
	rm src/c_lexer_ast.yy.cpp
	rm src/*.o
	rm src/*.s
