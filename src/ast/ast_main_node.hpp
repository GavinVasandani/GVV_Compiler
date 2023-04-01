#ifndef ast_main_node_hpp
#define ast_main_node_hpp

#include <iostream>
#include <string>
#include <vector>

//main node has { decl_list } then pntr inside it. It inherits from node and head is same type as node and its outputted. Head is form int main() main and output g_root is node
class main_node : public node {
    public:
        NodeListPtr ASTListPtr = new NodeList();
        //Main_node constructor that assigns nodeListPtr to input list:

        main_node(NodeListPtr _ASTListPtr) : ASTListPtr(_ASTListPtr) {}

        virtual void print(std::ostream& dst) {
            for (auto ptr : *ASTListPtr) {
                ptr->print(dst);
            }
        }

        virtual void stackBuild(stackAST &Map, std::vector<variable_state> &vec){
            for (auto ptr: *ASTListPtr) {
                ptr->stackBuild(Map, vec);
            }
        }

        virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
            for (auto ptr: *ASTListPtr) {
                ptr->riscv(dst, reg_ctxt, freg_ctxt, Map);
            }
        }

        virtual ~main_node() override {
            for (auto ptr : *ASTListPtr) {
                if(ptr!=NULL){ delete ptr; }
            }
        }
};

#endif
