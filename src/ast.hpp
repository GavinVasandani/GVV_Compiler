#ifndef ast_hpp
#define ast_hpp

//Write all header files for all statement types, expression types, declaration types in here.
//Create all these header files in AST folder
#include "ast/variable_state.hpp"
#include "ast/register_state.hpp"
#include "ast/float_register_state.hpp"
#include "ast/stack.hpp"
#include "ast/ast_node.hpp"
#include "ast/ast_node_list_func.hpp"
#include "ast/ast_main_node.hpp"
#include "ast/ast_type_node.hpp"
#include "ast/ast_param_node.hpp"
#include "ast/ast_expr_node.hpp"
#include "ast/ast_decl_node.hpp"
#include "ast/ast_stmnt_node.hpp"
#include "ast/ast_case_list_func.hpp"

extern node* parseAST(std::string filename);

extern stackAST varStack;

extern void CompileToRISCV(node program); // we give the g_root to the compile function to finish compilation

#endif
