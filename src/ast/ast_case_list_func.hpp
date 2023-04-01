#ifndef ast_case_list_func_hpp
#define ast_case_list_func_hpp

#include <iostream>
#include <string>
#include <vector>

//vector containing all nodes, so just AST in vector form.
//each block statement has a nodeVec with elements inside that we recurse through and call its print func. So create nodeVec object for each major object (block) etc.

//function called at last decl so this is where we create NodeList AST
inline CaseListPtr create_case_list(case_stmnt* line) {
    CaseListPtr case_tree = new CaseList(); 
    case_tree->push_back(line);
    return case_tree; //initializes list of nodes. Each node has print function that has member variables in terms of other nodes so no need for list for each statement. Just put all overall nodes in 1 body
}

inline CaseListPtr append_case_list(CaseListPtr case_tree, case_stmnt* line) {
    case_tree->push_back(line);
    return case_tree;
}

#endif