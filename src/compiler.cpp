#include <fstream>
#include <iostream>
#include <unistd.h>
#include "ast.hpp"

#include "cli.h"

void compile(std::ostream &w, std::string file)
{
    bool printAST = false;

    std::vector<variable_state> vec;
    stackAST varStack;

    node* ast = parseAST(file);
    //Compile function just runs entire compiler
    ast->stackBuild(varStack, vec);

    if (printAST) {
        ast->print(w);
    }
    else {
        register_context reg_ctxt;
        fregister_context freg_ctxt;
        ast->riscv(w, reg_ctxt, freg_ctxt, varStack);
    }
    //maybe just do ast->codegenfunc(w); to write RISCV funcs to file using codegenfunc method in each class

    //std::cout<<varStack.calcStack(varStack.getCurrFunc())<<std::endl;

    varStack.stackPrint();
    //std::cout<<varStack.calcStack("f")<<std::endl;
}

// TODO: uncomment the below if you're using Flex/Bison.
extern FILE *yyin;

int main(int argc, char **argv)
{ //intended input is: bin/c_compiler -S [source-file.c] -o [dest-file.s]
/*argc would be equal to 5, since there are 5 arguments passed to the program: "bin/c_compiler", "-S", [source-file.c], "-o", and [dest-file.s].
argv would be an array of strings:
argv[0] would be "bin/c_compiler"
argv[1] would be "-S"
argv[2] would be [source-file.c]
argv[3] would be "-o"
argv[4] would be [dest-file.s]*/
	if(argc < 5){
		std::cerr<<"Usage: bin/c_compiler -S [source-file.c] -o [dest-file.s] -V (optional)"<<std::endl;
		return 1;
	}

    // Parse CLI arguments, to fetch the values of the source and output files.
    std::string sourcePath = "";
    std::string outputPath = "";
    //not sure where parse_command_line_args func is defined
    if (parse_command_line_args(argc, argv, sourcePath, outputPath)) //parses through CLI args and assigns to argc, argv, sourcePath, outputPath
    //sourcePath is the c program (source) and outputPath is file we output results in.
    {
        return 1;
    }

    // TODO: uncomment the below lines if you're using Flex/Bison.
    // This configures Flex to look at sourcePath instead of
    // reading from stdin.
    const char* sourcePath_c = sourcePath.c_str(); //sourcePath is c programs, it's converted entire to a c-style string and checked if openable and if so assigned to yyin file which is inputted into lexer
    FILE* yyin = fopen(sourcePath_c, "r");
    if (yyin == NULL)
    {
        perror("Could not open source file");
        return 1;
    }

    // Open the output file in truncation mode (to overwrite the contents).
    //use this to write RISCV instructions to output file.
    //to test AST is correct, maybe output AST into this file.
    std::ofstream output;
    output.open(outputPath, std::ios::trunc); //assigns output as the file contain that text is written t

    // Compile the input
    std::cout << "Compiling: " << sourcePath << std::endl;
    compile(output, sourcePath_c);
    std::cout << "Compiled to: " << outputPath << std::endl;

    output.close();
    return 0;
}
