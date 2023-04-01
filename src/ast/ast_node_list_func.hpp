#ifndef ast_node_list_func_hpp
#define ast_node_list_func_hpp

#include <iostream>
#include <string>
#include <vector>

//vector containing all nodes, so just AST in vector form.
//each block statement has a nodeVec with elements inside that we recurse through and call its print func. So create nodeVec object for each major object (block) etc.

typedef std::vector<nodeptr> NodeList;
typedef NodeList* NodeListPtr;

//function called at last decl so this is where we create NodeList AST
inline NodeListPtr create_list(nodeptr line) {
    NodeListPtr AST = new NodeList(); 
    AST->push_back(line);
    return AST; //initializes list of nodes. Each node has print function that has member variables in terms of other nodes so no need for list for each statement. Just put all overall nodes in 1 body
}

inline NodeListPtr append_list(NodeListPtr AST, nodeptr line) {
    AST->push_back(line);
    return AST;
}

#endif