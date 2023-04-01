#ifndef ast_type_node_hpp
#define ast_type_node_hpp

#include <iostream>
#include <string>

//Implement subtypes later
//put NULL for params if not needed.
/*
typedef enum {
    IntType, DoubleType, LongType, ShortType, FloatType, CharType, StringType, VoidType, AnyType
} varType; */

class type_node : public node { //can just do one class, no need for children as type node has same implementation for all types
    private:
        //std::string type_val;
        //enum used over string as if not in enum then we can output unknown type error.
        varType kind;
        //type_node* subtype;
        //param_node* params;
    public:
        type_node(varType _kind) : //type_node* _subtype, param_node* _params) :
        kind(_kind)
        //subtype(_subtype),
        //param(_params)
        {}
        virtual void print(std::ostream& dst) override{
            if (kind==IntType) {dst<<"int";}
            if (kind==DoubleType) {dst<<"double";}
            if (kind==LongType) {dst<<"long";}
            if (kind==ShortType) {dst<<"short";}
            if (kind==FloatType) {dst<<"float";}
            if (kind==CharType) {dst<<"char";}
            if (kind==StringType) {dst<<"string";}
            if (kind==VoidType) {dst<<"void";}
            if (kind==UnsignedType) {dst<<"unsigned";}
            //dst<<type_val; //outputs string.
        }

        varType get_kind(){ return kind; }

        std::string getTypePrint() const {
            if (kind==IntType) {return "int";}
            if (kind==DoubleType) {return "double";}
            if (kind==LongType) {return "long";}
            if (kind==ShortType) {return "short";}
            if (kind==FloatType) {return "float";}
            if (kind==CharType) {return "char";}
            if (kind==StringType) {return "string";}
            if (kind==VoidType) {return "void";}
            if (kind==UnsignedType) {return "unsigned";}
        }

        virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
            //not sure what to put, maybe after assigning a variable we assign varType using type_node but how to know which varName we're assigning?
            //for now assume default IntType
        }

};


#endif
