#ifndef ast_node_hpp
#define ast_node_hpp

#include <iostream>
#include <vector>
#include <string>
#include "stack.hpp"

//General code for most base class:

class node;

typedef node* nodeptr;

static int makeNameUnq=0;

static std::string makeName(std::string base)
{
    return "_"+base+"_"+std::to_string(makeNameUnq++); //_ is just whitespace
}

//forward declare all the nodes here?

class node {

    public: //if access identifier (public, private) not mentioned then it assumes private

        std::string varName = "empty";
        bool store_flag = false;
        int currAReg = 0;
        int currAFReg = 0;
        std::vector<std::string> noResetRegs = {"ra"}; //not needed, remove all of it and where its used as input parameter when recursion starts working

        //Want destructor to be overwritten:
        virtual ~node () {}

        //Print function to be pure virtual so must always be defined in derived class:
        virtual void print(std::ostream &dst) =0;

        //purely virtual func so all derived classes need to rewrite riscv func which is just the riscv code that implements the class node.
        virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map){  };

        //Needed overloaded version for recursive functions
        virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map, bool store_flag, int currAReg, int currAFReg, std::vector<std::string>& noResetRegs) { };

        //Needed overloaded version for switch statements, doesn't have to be called riscv, function can be named something else
        virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map, std::string caseL) { };


        virtual void stackBuild(stackAST &Map, std::vector<variable_state> &vec){  }

        //defining stackBuild just for scoped_def:
        virtual void stackBuildScopeDef(stackAST &Map, std::vector<std::vector<variable_state>> &vec){  }

        std::string getVarName() {return varName;}
        std::string& setVarName() {return varName;}

};

#endif
