#ifndef variable_state_hpp
#define variable_state_hpp

#include <string>

//class that is used to give all the data/state regarding a locally declared variable.
//variable name, type, what register stored in, or what stack memory address stored in, stored in stack mem or reg.

//no need for #include<string> as header files in same folder have already imported it and header files in same folder are readily accessible alongside their imported libraries without needing #include

//Possible types for the variable:
typedef enum {
    IntType, DoubleType, LongType, ShortType, FloatType, CharType, StringType, VoidType, UnsignedType, AnyType //add UnSignedType
} varType;
//might have to define varType here and not in type_node due to ast includes order. Might have to remove varType enum from type node due to redefinition
class variable_state {
    private:
        std::string name; //variable identifier (name)
        varType kind; //type of variable
        std::string reg; //register assigned to variable
        int stack_mem_addr; //stack memory address assigned to variable if stored on stack
        bool in_stack_mem; //true if variable in stack mem, false if in register
        bool isPntr;  //default value
    public:
        //constructor:
        variable_state(std::string _name = "", varType _kind = AnyType, std::string _reg = "", bool _isPntr = false, int _stack_mem_addr = 0, bool _in_stack_mem = false) :
        name(_name), kind(_kind), reg(_reg), isPntr(_isPntr), stack_mem_addr(_stack_mem_addr), in_stack_mem(_in_stack_mem) {}

        //new variable location:
        void setlocation(std::string _reg, int _stack_mem_addr, bool _in_stack_mem) {
            reg = _reg;
            stack_mem_addr = _stack_mem_addr;
            in_stack_mem = _in_stack_mem;
        }

        //getter functions:
        std::string getName() const {return name;}
        varType getType() const {return kind;}
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
        std::string getReg() const {return reg;}
        int getStackMemAddr() const {return stack_mem_addr;}
        bool getInStackMem() const {return in_stack_mem;}
        bool getIsPntr() const {return isPntr;}

        //setter functions:
        std::string& setName() {return name;}
        varType& setType() {return kind;}
        std::string& setReg() {return reg;}
        int& setStackMemAddr() {return stack_mem_addr;}
        bool& setInStackMem() {return in_stack_mem;}
};

#endif
