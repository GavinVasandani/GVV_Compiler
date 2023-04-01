#ifndef ast_decl_node_hpp
#define ast_decl_node_hpp

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class decl_node : public node { //create child node for each type of decl_node (like function prototype, variable declaration) so print function can be made for each
    private: //member variables, need to be forward declared
        std::string name; //name of variable/func
        type_node* type; //its type specifier
        expr_node* value; //value assigned to variable
        param_node* param; //parameter of func

    protected: //constructor
        decl_node(std::string _name, type_node* _type, expr_node* _value, param_node* _param) :
        name(_name),
        type(_type),
        value(_value),
        param(_param)
        {}

    public:
        std::string get_name() {return name;}
        std::string& set_name() {return name;}
        type_node* get_type() {return type;} //function that returns type member variable
        expr_node* get_value() {return value;}
        param_node* get_param() {return param;}

        virtual std::vector<variable_state> callStack(stackAST& Map, std::string func_name){
            return Map.returnVars(func_name);
        }

        virtual int stackOffset(std::vector<variable_state> varList, std::string _name){
            for(variable_state var : varList){
                if(var.getName() == _name){
                    return var.getStackMemAddr();
                }
            }
            throw("variable not defined in scope");
            return -4;
        }

    //Destructor using RAII:
    //This might be an issue, if some decl type doesn't use a member variable then its NULL, then delete NULL might be problematic.
    //Might have to breakdown decl_node into something smaller.
    //Fix child class constructors. Give all member variables default values and only call
        virtual ~decl_node() override { //just put func declaration, then maybe define all destructors in 1 folder so there isn't error with declaration order {
            delete type;
            delete value;
            delete param;  // check this with the TAs, do we delete null pointers if these remain unassiged?
        }
}; //still abstract class as void print hasn't be overridden.

class var_decl : public decl_node {
    public:
    var_decl(std::string _name, type_node* _type, expr_node* _value, param_node* _param) :
    decl_node(_name, _type, _value, _param)
    {}

    //virtual void as we want to use this print func when upcasting
    virtual void print(std::ostream &dst) override {
        get_type()->print(dst);
        dst<<" "<<get_name();
        dst<<";"<<std::endl;
    }

    virtual void stackBuild(stackAST &Map, std::vector<variable_state> &vec){
        Map.incVector(vec, get_name(), get_type()->get_kind());
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {

        auto func_name = Map.getCurrFunc(); //string storing current func_name
        //Variable declaration for int type:

        if(get_type()->getTypePrint()=="int") {
            reg_ctxt.newVar(get_name(), get_type()->get_kind(), "a0", "a7"); //variable that is assigned to reg has name get_name()
            dst<<"li "<<reg_ctxt.findVarReg(get_name())<<", 0"<<std::endl; //loads default value 0 into register
            dst<<"sw "<<reg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-4<<"(sp)"<<std::endl;
            reg_ctxt.emptyReg(get_name()); //empties register that was used to store variable.
        }
        else if(get_type()->getTypePrint()=="double") { //load into float register
            auto func_name = Map.getCurrFunc();
            std::string temp_zeroreg = makeName("reg0");
            reg_ctxt.newVar(temp_zeroreg, IntType, "a0", "a7");
            dst<<"li "<<reg_ctxt.findVarReg(temp_zeroreg)<<", 0"<<std::endl; //loads default value 0 into register, needed as zero reg doesnt exist in floats

            //Load this into freg
            freg_ctxt.newVar(get_name(), get_type()->get_kind(), "fa0", "fa7");
            dst<<"fmv.d.x "<<freg_ctxt.findVarReg(get_name())<<", "<<reg_ctxt.findVarReg(temp_zeroreg)<<std::endl;
            //Loading this into memory
            dst<<"fsd "<<freg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-8<<"(sp)"<<std::endl;
            freg_ctxt.emptyReg(get_name());
            reg_ctxt.emptyReg(temp_zeroreg);
        }
        else if(get_type()->getTypePrint()=="float") {
            auto func_name = Map.getCurrFunc();
            std::string temp_zeroreg = makeName("reg0");
            reg_ctxt.newVar(temp_zeroreg, IntType, "a0", "a7");
            dst<<"li "<<reg_ctxt.findVarReg(temp_zeroreg)<<", 0"<<std::endl; //loads default value 0 into register, needed as zero reg doesnt exist in floats

            //Load this into freg
            freg_ctxt.newVar(get_name(), get_type()->get_kind(), "fa0", "fa7");
            dst<<"fmv.w.x "<<freg_ctxt.findVarReg(get_name())<<", "<<reg_ctxt.findVarReg(temp_zeroreg)<<std::endl;
            //Loading this into memory
            dst<<"fsw "<<freg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-4<<"(sp)"<<std::endl;
            freg_ctxt.emptyReg(get_name());
            reg_ctxt.emptyReg(temp_zeroreg);
        }
        else {
            reg_ctxt.newVar(get_name(), get_type()->get_kind(), "a0", "a7"); //variable that is assigned to reg has name get_name()
            dst<<"li "<<reg_ctxt.findVarReg(get_name())<<", 0"<<std::endl; //loads default value 0 into register
            auto func_name = Map.getCurrFunc(); //string storing current func_name
            dst<<"sw "<<reg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-4<<"(sp)"<<std::endl;
            reg_ctxt.emptyReg(get_name()); //empties register that was used to store variable.
        }
    }
};

//variable definition: int a = 5; just outputting not assigning bindings for now
class var_def : public decl_node {
    public:
    var_def(std::string _name, type_node* _type, expr_node* _value, param_node* _param) :
    decl_node(_name, _type, _value, _param)
    {}

    virtual void print(std::ostream &dst) override {
        //expected print: int a = 5;
        get_type()->print(dst);
        dst<<" "<<get_name()<<" ";
        dst<<"= ";
        get_value()->print(dst); //value can be any expression so: int a = 5 + 5;
        dst<<";"<<std::endl;
    }

    virtual void stackBuild(stackAST &Map, std::vector<variable_state> &vec){
        Map.incVector(vec, get_name(), get_type()->get_kind());
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {        //int a; //default consider variable is IntType


        if(get_type()->getTypePrint()=="int") {

            //assign new register to variable (use t0-t6 regs, not sure about these ranges)
            get_value()->riscv(dst, reg_ctxt, freg_ctxt, Map);

            reg_ctxt.newVar(get_name(), get_type()->get_kind(), "a0", "a7");
            //creates variable with varName = get_name() in a free register (t0-t6)
            //outputs the register name that stores this variable: findVarReg(get_name())

            //stores int literal to register that represents variable, now store variable in stack and free reg.
            dst<<"add "<<reg_ctxt.findVarReg(get_name())<<", "<<"zero"<<", "<<reg_ctxt.findVarReg(get_value()->getVarName())<<std::endl;

            auto func_name = Map.getCurrFunc(); //string storing current func_name
            dst<<"sw "<<reg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-4<<"(sp)"<<std::endl;
            reg_ctxt.emptyReg(get_name());

        }

        else if(get_type()->getTypePrint()=="double") {
            get_value()->riscv(dst, reg_ctxt, freg_ctxt, Map);
            freg_ctxt.newVar(get_name(), get_type()->get_kind(), "fa0", "fa7");
            dst<<"fmv.d "<<freg_ctxt.findVarReg(get_name())<<", "<<freg_ctxt.findVarReg(get_value()->getVarName())<<std::endl;

            //store in memory
            auto func_name = Map.getCurrFunc();
            dst<<"fsd "<<freg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-8<<"(sp)"<<std::endl;

            //empty float reg:
            freg_ctxt.emptyReg(get_name());
        }

        else if(get_type()->getTypePrint()=="float") {
            get_value()->riscv(dst, reg_ctxt, freg_ctxt, Map);
            freg_ctxt.newVar(get_name(), get_type()->get_kind(), "fa0", "fa7");
            dst<<"fmv.s "<<freg_ctxt.findVarReg(get_name())<<", "<<freg_ctxt.findVarReg(get_value()->getVarName())<<std::endl;

            //store in memory
            auto func_name = Map.getCurrFunc();
            dst<<"fsw "<<freg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-4<<"(sp)"<<std::endl;

            //empty float reg:
            freg_ctxt.emptyReg(get_name());
        }
        else {
            get_value()->riscv(dst, reg_ctxt, freg_ctxt, Map);

            reg_ctxt.newVar(get_name(), get_type()->get_kind(), "a0", "a7");
            //creates variable with varName = get_name() in a free register (t0-t6)
            //outputs the register name that stores this variable: findVarReg(get_name())

            //stores int literal to register that represents variable, now store variable in stack and free reg.
            dst<<"add "<<reg_ctxt.findVarReg(get_name())<<", "<<"zero"<<", "<<reg_ctxt.findVarReg(get_value()->getVarName())<<std::endl;

            auto func_name = Map.getCurrFunc(); //string storing current func_name
            dst<<"sw "<<reg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-4<<"(sp)"<<std::endl;
            reg_ctxt.emptyReg(get_name());
        }

    }

};

class func_decl : public decl_node {
    public:
    func_decl(std::string _name, type_node* _type, expr_node* _value, param_node* _param) :
    decl_node(_name, _type, _value, _param)
    {}

    virtual void print(std::ostream& dst) override {
        //expected print: int a (int x);
        if(get_type()!=NULL) {get_type()->print(dst);}
        dst<<" "<<get_name()<<" ";
        dst<<"(";
        if(get_param()!=NULL) {get_param()->print(dst);} //this prints argument list.}, allows us to use func_decl even if no args as then NULL so no param is outputted
        dst<<")";
        dst<<";"<<std::endl;
    }

    virtual void stackBuild(stackAST &Map, std::vector<variable_state> &vecUnused){
        auto vec = std::vector<std::vector<variable_state>>();
        auto varList = std::vector<variable_state>();
        //use overloaded stackBuild func for param, so that it uses same varList
        if(get_param()!= NULL){ get_param()->stackBuild(Map, varList); } //allocate stack memory for input params

        //add this varList to scopeVarList
        Map.addScopeVarList(vec, varList);

        Map.resetStack();
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {        //function declaration has a definition somewhere else which implements instruction code for the function.
        Map.setCurrFunc(get_name());
        //function declaration just created .globl label
        //dst<<".globl "<<get_name()<<std::endl; //func name is used as label
        //dst<<"jr ra"<<std::endl;
    }

};

//function definition: int a (int x) { return x + 2; }
class func_def : public decl_node {
    private:
    decl_node* func_scope;
    public:
    func_def(std::string _name, type_node* _type, expr_node* _value, param_node* _param, decl_node* _func_scope) :
    decl_node(_name, _type, _value, _param), func_scope(_func_scope)
    {}

    virtual void print(std::ostream& dst) override {
        //expected print: int a (int x) { return x + 2; }
        if(get_type()!=NULL) {get_type()->print(dst);}
        dst<<" "<<get_name()<<" ";
        dst<<"(";
        if(get_param()!=NULL) {get_param()->print(dst);}
        dst<<")";
        func_scope->print(dst);
    }

    virtual void stackBuild(stackAST &Map, std::vector<variable_state> &vecUnused){
        //vec.clear(); //scopesVarList is cleared when new function is defined.
        //no need for vec.clear just reinitialize a new vector:
        auto vec = std::vector<std::vector<variable_state>>();
        //params are all in 1 scope level
        auto varList = std::vector<variable_state>();
        //use overloaded stackBuild func for param, so that it uses same varList

        func_scope->stackBuildScopeDef(Map, vec); //scopeVarList is being filled based on scopes

        //add this varList to scopeVarList
        if(get_param()!= NULL){ get_param()->stackBuild(Map, varList); } //allocate stack memory for input params
        Map.addScopeVarList(vec, varList); //so input params is last in varList so most global scope

        //std::string tempStackMem = makeName("tempStackMem");

        //Add these temporary variables to first varList, so vec[0]

        //we want to put temporary variables in last varList, 1st varList is most local scope.

        std::string tempStackMem = "tempStackMem";
        Map.incVector(vec.back(), tempStackMem, IntType);

        std::string rega1 = "rega1";
        Map.incVector(vec.back(), rega1, IntType);

        std::string rega2 = "rega2";
        Map.incVector(vec.back(), rega2, IntType);

        std::string rega3 = "rega3";
        Map.incVector(vec.back(), rega3, IntType);

        std::string rega4 = "rega4";
        Map.incVector(vec.back(), rega4, IntType);

        std::string rega5 = "rega5";
        Map.incVector(vec.back(), rega5, IntType);

        std::string rega6 = "rega6";
        Map.incVector(vec.back(), rega6, IntType);

        std::string rega7 = "rega7";
        Map.incVector(vec.back(), rega7, IntType);

        Map.addBinding(get_name(), vec);

        Map.setCurrFunc(get_name()); //setting currFunc name to func_def we're in.

        Map.resetStack();

    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        int x = Map.calcStack(get_name());
        Map.setCurrFunc(get_name());
        //int x = 0; // comment this out when running

        //function definition: int a (int x) { return x + 2; }, so cpp code below this is the function code, so must start with label and then print rest of body code
        //store return address. //not executing a function call so no need for this.
        //create label.
        //jump back to return address. //just writing the function code with label no need for mump
        dst<<".globl "<<get_name()<<std::endl; //func name is used as label

        dst<<get_name()<<":"<<std::endl; //func name is used as label
        dst<<"addi sp, sp, "<<-x<<std::endl;
        // dst<<"sw ra, "<<x-4<<"(sp)"<<std::endl; // do i uncomment this?
        dst<<"sw ra, "<<Map.lookUpVarStackAddr(get_name(), "tempStackMem")-4<<"(sp)"<<std::endl;
        //store all args first in regs
        store_flag = true;
        if(get_param()!=NULL) {get_param()->riscv(dst, reg_ctxt, freg_ctxt, Map, store_flag, 0, 0, noResetRegs);}

        func_scope->riscv(dst, reg_ctxt, freg_ctxt, Map);

        dst<<"addi sp, sp, "<<x<<std::endl;

    }

};

class array_decl : public decl_node {
    public:
        array_decl(std::string _name, type_node* _type, expr_node* _value, param_node* _param) :
        decl_node(_name, _type, _value, _param)
        {}
        // "_value" is probably required for {}, ie assignment. Need to make sure I cover {} assignment as well as indexed assignment.

        virtual void print (std::ostream& dst) override {
            // expected print "int x[8];"
            get_type()->print(dst);
            dst<<" "<<get_name()<<"[";
            get_value()->print(dst); // note that in this case you only need one param which is a number.
            dst<<"];"<<std::endl;
        }

        virtual void stackBuild(stackAST &Map, std::vector<variable_state> &vec){
            Map.incVector(vec, get_name(), get_type()->get_kind(), false, get_value()->get_int_val());
        }

        virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map){

            auto func_name = Map.getCurrFunc(); //string storing current func_name

        // //Variable declaration for int type:
            if(func_name != ""){
        //         if(get_type()->getTypePrint()=="int") {
        //             reg_ctxt.newVar(get_name(), get_type()->get_kind(), "a0", "a7"); //variable that is assigned to reg has name get_name()
        //             for(int i = 0; i < get_value()->get_int_val(); i++){
        //                 dst<<"li "<<reg_ctxt.findVarReg(get_name())<<", 0"<<std::endl; //loads default value 0 into register
        //                 dst<<"sw "<<reg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-4<<"(sp)"<<std::endl;
        //             }
        //             reg_ctxt.emptyReg(get_name()); //empties register that was used to store variable.
        //         }
        //         else if(get_type()->getTypePrint()=="double") { //load into float register
        //             std::string temp_zeroreg = makeName("reg0");
        //             reg_ctxt.newVar(temp_zeroreg, IntType, "a0", "a7");
        //             dst<<"li "<<reg_ctxt.findVarReg(temp_zeroreg)<<", 0"<<std::endl; //loads default value 0 into register, needed as zero reg doesnt exist in floats

        //             //Load this into freg
        //             freg_ctxt.newVar(get_name(), get_type()->get_kind(), "fa0", "fa7");
        //             for(int i = 0; i < get_value()->get_int_val(); i++){
        //                 dst<<"fmv.d.x "<<freg_ctxt.findVarReg(get_name())<<", "<<reg_ctxt.findVarReg(temp_zeroreg)<<std::endl;
        //                 //Loading this into memory
        //                 dst<<"fsd "<<freg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-8<<"(sp)"<<std::endl;
        //             }
        //             freg_ctxt.emptyReg(get_name());
        //             reg_ctxt.emptyReg(temp_zeroreg);
        //         }
        //         else if(get_type()->getTypePrint()=="float") {
        //             std::string temp_zeroreg = makeName("reg0");
        //             reg_ctxt.newVar(temp_zeroreg, IntType, "a0", "a7");
        //             dst<<"li "<<reg_ctxt.findVarReg(temp_zeroreg)<<", 0"<<std::endl; //loads default value 0 into register, needed as zero reg doesnt exist in floats

        //             //Load this into freg
        //             freg_ctxt.newVar(get_name(), get_type()->get_kind(), "fa0", "fa7");
        //             for(int i = 0; i < get_value()->get_int_val(); i++){
        //                 dst<<"fmv.w.x "<<freg_ctxt.findVarReg(get_name())<<", "<<reg_ctxt.findVarReg(temp_zeroreg)<<std::endl;
        //                 //Loading this into memory
        //                 dst<<"fsw "<<freg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-4<<"(sp)"<<std::endl;
        //             }
        //             freg_ctxt.emptyReg(get_name());
        //             reg_ctxt.emptyReg(temp_zeroreg);
        //         }
        //         else {
        //             reg_ctxt.newVar(get_name(), get_type()->get_kind(), "a0", "a7"); //variable that is assigned to reg has name get_name()
        //             for(int i = 0; i < get_value()->get_int_val(); i++){
        //                 dst<<"li "<<reg_ctxt.findVarReg(get_name())<<", 0"<<std::endl; //loads default value 0 into register //string storing current func_name
        //                 dst<<"sw "<<reg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-4<<"(sp)"<<std::endl;
        //             }
        //             reg_ctxt.emptyReg(get_name()); //empties register that was used to store variable.
        //         }
            }

        }
};

class array_def : public decl_node {
    public:
        array_def(std::string _name, type_node* _type, expr_node* _value, param_node* _param) :
        decl_node(_name, _type, _value, _param)
        {}
        // "_value" is probably required for {}, ie assignment. Need to make sure I cover {} assignment as well as indexed assignment.

        // if_value == NULL means int x[]={}

        virtual void print (std::ostream& dst) override {
            // expected print "int x[8] = {array elements here};"
            // array elements are stored as parameters
            get_type()->print(dst);
            dst<<" "<<get_name()<<"[";
            if(get_value()!=NULL){get_value()->print(dst);}
            dst<<"]";
            dst<< "= {";
            get_param()->print(dst);
            dst<<"};"<<std::endl;
        }

        virtual void stackBuild(stackAST &Map, std::vector<variable_state> &vec){
            int count = 0;
            if(get_value()!=NULL){
                count = get_value()->get_int_val();
            }
            else{
                count = get_param()->count_params(0);
            }
            Map.incVector(vec, get_name(), get_type()->get_kind(), false, count);
        }

        virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map){
            auto func_name = Map.getCurrFunc();
        }
};

class scope_def : public decl_node {
    private:
        NodeListPtr SubTreeListPtr = new NodeList();
    public:
        scope_def(std::string _name, type_node* _type, expr_node* _value, param_node* _param, NodeListPtr _SubTreeListPtr) :
        decl_node(_name, _type, _value, _param), SubTreeListPtr(_SubTreeListPtr)
        {}

        virtual void print (std::ostream& dst) override {
            dst<<"{"<<std::endl;

            if(SubTreeListPtr!=NULL) {
                for (auto ptr : *SubTreeListPtr) { //does empty function body case exist?
                    ptr->print(dst);
                }
            }
            dst<<"}"<<std::endl;
        }

        virtual void stackBuildScopeDef(stackAST& Map, std::vector<std::vector<variable_state>>& vec) {

            //everytime there's a new scope def we intiailize a new varList:
            auto varList = std::vector<variable_state>();
            for (auto ptr : *SubTreeListPtr) { //allocate stack memory for variables in body of func.
                ptr->stackBuild(Map, varList); //but if ptr is a scoped_statement then as scoped_statement doesn't use stackBuild func then it ignores it

            //we need a flag for a scoped_statement
            //or can do ptr->stackBuildScopeDef(Map, vec) as it's only defined for stack_statement then its ignored for all other node types

                ptr->stackBuildScopeDef(Map, vec); //the higher the scope will be first in the scope vector, so need to make temp variables in last varList

            }
            //at end of scoped def we insert into vec aka scopedVarList

            Map.addScopeVarList(vec, varList);
        }

        virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
            if(SubTreeListPtr!=NULL) {
                for (auto ptr : *SubTreeListPtr) { //specifies instructions inside function definition.
                    ptr->riscv(dst, reg_ctxt, freg_ctxt, Map);
                }
            }
        }
};

class pntr_decl : public decl_node {
    public: //int *y;
    pntr_decl(std::string _name, type_node* _type, expr_node* _value, param_node* _param) :
    decl_node(_name, _type, _value, _param)
    {}

    //virtual void as we want to use this print func when upcasting
    virtual void print(std::ostream &dst) override {
        get_type()->print(dst);
        dst<<" *"<<get_name();
        dst<<";"<<std::endl;
    }

    virtual void stackBuild(stackAST &Map, std::vector<variable_state> &vec){
        Map.incVector(vec, get_name(), get_type()->get_kind(), true);
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {

        auto func_name = Map.getCurrFunc();

        if(get_type()->getTypePrint()=="int") { //intiailizes pointer to 0 and stores in memory.
            reg_ctxt.newVar(get_name(), get_type()->get_kind(), "a0", "a7", true); //variable that is assigned to reg has name get_name()
            dst<<"li "<<reg_ctxt.findVarReg(get_name())<<", 0"<<std::endl; //loads default value 0 into register
            dst<<"sw "<<reg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-4<<"(sp)"<<std::endl;
            reg_ctxt.emptyReg(get_name()); //empties register that was used to store variable.
        }

        else if(get_type()->getTypePrint()=="char") {
            reg_ctxt.newVar(get_name(), get_type()->get_kind(), "a0", "a7", true); //variable that is assigned to reg has name get_name()
            dst<<"li "<<reg_ctxt.findVarReg(get_name())<<", 0"<<std::endl; //loads default value 0 into register
            dst<<"sw "<<reg_ctxt.findVarReg(get_name())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-4<<"(sp)"<<std::endl;
            reg_ctxt.emptyReg(get_name()); //empties register that was used to store variable.

        }
    }
};

class pntr_def : public decl_node {
    public: //value is &x;
    pntr_def(std::string _name, type_node* _type, expr_node* _value, param_node* _param) :
    decl_node(_name, _type, _value, _param)
    {}

    virtual void print(std::ostream &dst) override {
        //expected print: int *y = &x;
        get_type()->print(dst);
        dst<<" *"<<get_name()<<" ";
        dst<<"= ";
        get_value()->print(dst);
        dst<<";"<<std::endl;
    }

    virtual void stackBuild(stackAST &Map, std::vector<variable_state> &vec){
        if(get_value()->get_string_val()=="0") {
            Map.incVector(vec, get_name(), get_type()->get_kind(), true); //variable binding has isPntr = true
        }
        else {
            int count = 0;
            count = get_value()->get_string_val().length()-2;
            //REMEMBER TO CHANGE FROM INTYPE TO CHARTYPE
            Map.incVector(vec, get_name(), get_type()->get_kind(), false, count);
        }
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {

        auto func_name = Map.getCurrFunc();

        if(get_value()->get_string_val()=="0") {
            //storing address of x in pointer y:
            get_value()->riscv(dst, reg_ctxt, freg_ctxt, Map);
            //storing &x into stack for int *y and emptying register:
            dst<<"sw "<<reg_ctxt.findVarReg(get_value()->getVarName())<<", "<<Map.lookUpVarStackAddr(func_name, get_name())-4<<"(sp)"<<std::endl; //maybe modify for chars
            //Empty register:
            reg_ctxt.emptyReg(get_value()->getVarName());
        }
        //else {
            //char *x = "hello"

        //TASK
        //Load ASCII values of string into array

        //Go from starting of x, so head of array till x + 5, load each character in string into each memory

        std::string head = makeName("head");
        reg_ctxt.newVar(head, get_type()->get_kind(), "a0", "a7", false);

        //loads address of head.
        dst<<"li "<<reg_ctxt.findVarReg(head)<<", "<<Map.lookUpVarStackAddr(func_name, get_name())<<std::endl;

        dst<<"add "<<reg_ctxt.findVarReg(head)<<", sp, "<<reg_ctxt.findVarReg(head)<<std::endl;

        //reverse(get_value()->get_string_val().begin(), get_value()->get_string_val().end());

        std::string val_string = get_value()->get_string_val();

        //for(auto i: val_string) {

        for(int i=1; i<val_string.length()-1; i++) {

            if(val_string[i]!='"') {
                if(val_string[i]!='\\') {
                std::string char1 = makeName("char1");
                reg_ctxt.newVar(char1, get_type()->get_kind(), "a0", "a7", false);
                //subtract head by 1
                dst<<"addi "<<reg_ctxt.findVarReg(head)<<", "<<reg_ctxt.findVarReg(head)<<", -4"<<std::endl;

                //dst<<"li "<<reg_ctxt.findVarReg(char1)<<", "<<int(get_value()->get_string_val().at(get_value()->get_string_val().length()-i-1))<<std::endl;
                //dst<<"li "<<reg_ctxt.findVarReg(char1)<<", "<<int(val_string.at(i))<<std::endl;

                dst<<"li "<<reg_ctxt.findVarReg(char1)<<", '"<<val_string.at(i)<<"'"<<std::endl;

                //dst<<"li "<<reg_ctxt.findVarReg(char1)<<", '"<<val_string[val_string.length()-i-1]<<"'"<<std::endl;

                dst<<"sw "<<reg_ctxt.findVarReg(char1)<<", 0("<<reg_ctxt.findVarReg(head)<<")"<<std::endl;

                reg_ctxt.emptyReg(char1);
                }

                else {
                    std::string char1 = makeName("char1");
                    reg_ctxt.newVar(char1, get_type()->get_kind(), "a0", "a7", false);
                    //subtract head by 1
                    dst<<"addi "<<reg_ctxt.findVarReg(head)<<", "<<reg_ctxt.findVarReg(head)<<", -4"<<std::endl;

                    //dst<<"li "<<reg_ctxt.findVarReg(char1)<<", "<<int(get_value()->get_string_val().at(get_value()->get_string_val().length()-i-1))<<std::endl;
                    //dst<<"li "<<reg_ctxt.findVarReg(char1)<<", "<<int(val_string.at(i))<<std::endl;

                    dst<<"li "<<reg_ctxt.findVarReg(char1)<<", '\\"<<val_string.at(i)<<"'"<<std::endl;

                    //dst<<"li "<<reg_ctxt.findVarReg(char1)<<", '"<<val_string[val_string.length()-i-1]<<"'"<<std::endl;

                    dst<<"sw "<<reg_ctxt.findVarReg(char1)<<", 0("<<reg_ctxt.findVarReg(head)<<")"<<std::endl;

                    reg_ctxt.emptyReg(char1);

                }

            }
        }

    }
};

class struct_decl : public decl_node{
    private:
    std::string declname;
    public:
    struct_decl(std::string _name, type_node* _type, expr_node* _value, param_node* _param, std::string _declname) :
    decl_node(_name, _type, _value, _param), declname(_declname)
    {}

    virtual void print(std::ostream& dst) override {

    }
    // params store the variables like int x, int y and all of that. Can use arguments and then ';'. This will use type_specifier and IDENTIFIER up


};

#endif