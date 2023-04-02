#ifndef ast_expr_node_hpp
#define ast_expr_node_hpp

#include <iostream>
#include <string>
#include <cstdint>

class expr_node : public node { //create child node for each type of stmnt_node: add, sub, mul, assign, ident, int_literal, string_literal
//make into smaller classes i.e. operators class, so don't have so many input params that aren't used that need to be set to NULL.
    private: //default values
        expr_node* left; //arithmetic operation is expression. this operation involves left operand and right operand
        expr_node* right;
        std::string name; //variable identifier we're referencing
        int int_val; //value of integer literal
        std::string string_val; //value of string literal
        //std::string varName = "empty";
    protected:
        expr_node(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        left(_left),
        right(_right),
        name(_name),
        int_val(_int_val),
        string_val(_string_val)
        {}

    public:
        std::string deref_reg;
        bool deref_flag = false;
        bool is_func_call = false; //flag to see if operand is a func_call
        int add_specifier = 1; //variable that states how much to add for a variable based on its pntr type.
        expr_node* getLeft() {return left;} //function that base class member variable
        expr_node* getRight() {return right;} //function that base class member variable
        std::string getName() {return name;}
        int get_int_val() {return int_val;}
        std::string get_string_val() {return string_val;}
        //std::string getVarName() {return varName;}
        //std::string& setVarName() {return varName;}

        virtual int callStack(const std::vector<variable_state> &vec, std::string varName){
            for(int i = 0; i < vec.size(); i++){
                if(vec[i].getName()==varName){
                    return vec[i].getStackMemAddr();
                }
            }
            throw("variable not in stack!");
            return -4;
        }

        virtual ~expr_node() {
            if(left!=NULL){  delete left; } // added failsafe for seg faults. Check with TA to confirm
            if(right!=NULL){ delete right; }
        }
};

//Add operation:
class expr_add : public expr_node {
    public:
        expr_add(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override{
        //output: 1 + 4;
        getLeft()->print(dst);
        dst<<"+";
        getRight()->print(dst);
    } //parent class destructor used

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {        //executes instructions for LHS, RHS operands.
        if(getRight()->is_func_call) { //if RHS is func_call do this
            getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);
            getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        }
        else {
            getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
            getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        }

        auto func_name = Map.getCurrFunc();

        //need to check if left or right is Pntr, if so then add to stack offset the shift based on the pointer type:
        //Maybe check if findVarReg of getLeft is register that stores pointer variable

        if(reg_ctxt.isVarInReg(getLeft()->getVarName())) { //we won't have case of int + double, so just need to check type of one of the operands.

            if(reg_ctxt.isVarPntr(getLeft()->getVarName()) == true) {
                add_specifier = getLeft()->add_specifier;

                //Load this add_specifier into a register:
                std::string add_spec_reg = makeName("add_spec");
                reg_ctxt.newVar(add_spec_reg, IntType, "t0", "t6");

                //not sure if this add_spec_reg is 0 - actually its solved now
                dst<<"addi "<<reg_ctxt.findVarReg(add_spec_reg)<<", zero, "<<add_specifier<<std::endl;
                //Multiply value in this register by the value in the getRight reg: so that we're adding by size of pointer type
                dst<<"mul "<<reg_ctxt.findVarReg(getRight()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<", "<<reg_ctxt.findVarReg(add_spec_reg)<<std::endl;

                //need to empty reg:
                reg_ctxt.emptyReg(add_spec_reg);
            }
            //Could load add_specifier in register and multiply getRight->getVarname by add_specifier

            dst<<"add "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            setVarName() = getLeft()->getVarName(); //so output of a + b is stored in a and that's assigned to varName so it can be called.
            //Empty RHS reg as sum is stored in LHS reg.
        }

        else if( freg_ctxt.isVarInReg(getLeft()->getVarName()) && freg_ctxt.VarType(getLeft()->getVarName())=="double" ) { //if operands are double
            dst<<"fadd.d "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            setVarName() = getLeft()->getVarName();
        }

        else if( freg_ctxt.isVarInReg(getLeft()->getVarName()) && freg_ctxt.VarType(getLeft()->getVarName())=="float" ) { //if operands are float
            dst<<"fadd.s "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            setVarName() = getLeft()->getVarName();
        }
    }
};

class expr_sub : public expr_node {
    public:
        expr_sub(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override{
        getLeft()->print(dst);
        dst<<"-";
        getRight()->print(dst);
    }
    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {        //executes instructions for LHS, RHS operands.
        //Assuming left is identifier:
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //Assuming right is constant:
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map); //li t0, int_literal

        if(reg_ctxt.isVarInReg(getLeft()->getVarName())) {
            //if true then do normal add
            dst<<"sub "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            setVarName() = getLeft()->getVarName(); //so output of a + b is stored in a and that's assigned to varName so it can be called.
            //Empty RHS reg as sum is stored in LHS reg.
        }

        else if( freg_ctxt.isVarInReg(getLeft()->getVarName()) && freg_ctxt.VarType(getLeft()->getVarName())=="double" ) { //if operands are double
            dst<<"fsub.d "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            setVarName() = getLeft()->getVarName();
        }

        else if( freg_ctxt.isVarInReg(getLeft()->getVarName()) && freg_ctxt.VarType(getLeft()->getVarName())=="float" ) { //if operands are float
            dst<<"fsub.s "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            setVarName() = getLeft()->getVarName();
        }
    }
};


class expr_mul : public expr_node {
    public:
        expr_mul(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override{
        //output: 1 + 4;
        getLeft()->print(dst);
        dst<<"*";
        getRight()->print(dst);
    } //parent class destructor used

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {        //executes instructions for LHS, RHS operands.
        //Assuming left is identifier:
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //Assuming right is constant:
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map); //li t0, int_literal

        if(reg_ctxt.isVarInReg(getLeft()->getVarName())) { //we won't have case of int + double, so just need to check type of one of the operands.
        //if true then do normal add
            dst<<"mul "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            setVarName() = getLeft()->getVarName(); //so output of a + b is stored in a and that's assigned to varName so it can be called.
            //Empty RHS reg as sum is stored in LHS reg.
        }

        else if( freg_ctxt.isVarInReg(getLeft()->getVarName()) && freg_ctxt.VarType(getLeft()->getVarName())=="double" ) { //if operands are double
            dst<<"fmul.d "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            setVarName() = getLeft()->getVarName();
        }

        else if( freg_ctxt.isVarInReg(getLeft()->getVarName()) && freg_ctxt.VarType(getLeft()->getVarName())=="float" ) { //if operands are float
            dst<<"fmul.s "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            setVarName() = getLeft()->getVarName();
        }
    }
};

class expr_div : public expr_node {
    public:
        expr_div(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override{
        //output: 1 + 4;
        getLeft()->print(dst);
        dst<<"/";
        getRight()->print(dst);
    } //parent class destructor used

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {        //executes instructions for LHS, RHS operands.
        //Assuming left is identifier:
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //Assuming right is constant:
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map); //li t0, int_literal
        //dst<<"div "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
        //setVarName() = getLeft()->getVarName(); //so output of a + b is stored in a and that's assigned to varName so it can be called.
    
        if(reg_ctxt.isVarInReg(getLeft()->getVarName())) { //we won't have case of int + double, so just need to check type of one of the operands.
        //if true then do normal div
            dst<<"div "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            setVarName() = getLeft()->getVarName(); //so output of a + b is stored in a and that's assigned to varName so it can be called.
            //Empty RHS reg as sum is stored in LHS reg.
        }

        else if( freg_ctxt.isVarInReg(getLeft()->getVarName()) && freg_ctxt.VarType(getLeft()->getVarName())=="double" ) { //if operands are double
            dst<<"fdiv.d "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            setVarName() = getLeft()->getVarName();
        }

        else if( freg_ctxt.isVarInReg(getLeft()->getVarName()) && freg_ctxt.VarType(getLeft()->getVarName())=="float" ) { //if operands are float
            dst<<"fdiv.s "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            setVarName() = getLeft()->getVarName();
        }
    }
};

class expr_neg : public expr_node {
    public: //right is the expression: -(expression)
        expr_neg(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override{
        //output: 1 + 4;
        dst<<"-";
        getRight()->print(dst);
    } //parent class destructor used

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {        //executes instructions for LHS, RHS operands.
        //Assuming right is expression:
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);

        std::string negVal = makeName("negVal");

        reg_ctxt.newVar(negVal, IntType , "t0", "t6"); //freeReg found and stored in it variable of name getName()
        dst<<"li "<<reg_ctxt.findVarReg(negVal)<<", -1"<<std::endl;

        dst<<"mul "<<reg_ctxt.findVarReg(getRight()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<", "<<reg_ctxt.findVarReg(negVal)<<std::endl;
        setVarName() = getRight()->getVarName(); //so output of a + b is stored in a and that's assigned to varName so it can be called.
    }
};

class expr_incr : public expr_node {
    public: //_left is the expr we're incrementing
        expr_incr(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: "dog" (string literal)
        //dst<<" \" ";
        getLeft()->print(dst);
        dst<<"++";
        //dst<<" \" "; //might be problem with these quotation marks
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        dst<<"addi "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", 1"<<std::endl;
        setVarName() = getLeft()->getVarName();
        //store in memory the updated value
        auto func_name = Map.getCurrFunc();
        int scope_index = Map.getCurrScopeIndex();
        
        dst<<"sw "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<Map.lookUpVarStackAddr(func_name, getLeft()->getVarName(), scope_index)-4<<"(sp)"<<std::endl;
    }
};

class expr_incr_cust : public expr_node {
    public: //_left is the expr we're incrementing, //right is amount we're incrementing by
        expr_incr_cust(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: "dog" (string literal)
        //dst<<" \" ";
        getLeft()->print(dst);
        dst<<"+=";
        getRight()->print(dst);
        dst<<std::endl;
        //dst<<" \" "; //might be problem with these quotation marks
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);

        dst<<"add "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;

        setVarName() = getLeft()->getVarName();
        //store in memory the updated value
        auto func_name = Map.getCurrFunc();
        dst<<"sw "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<Map.lookUpVarStackAddr(func_name, getLeft()->getVarName())-4<<"(sp)"<<std::endl;
    }
};

class expr_decr : public expr_node {
    public: //_left is the expr we're incrementing
        expr_decr(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: "dog" (string literal)
        //dst<<" \" ";
        getLeft()->print(dst);
        dst<<"--";
        //dst<<" \" "; //might be problem with these quotation marks
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        dst<<"addi "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", -1"<<std::endl;
        setVarName() = getLeft()->getVarName();
        auto func_name = Map.getCurrFunc();
        dst<<"sw "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<Map.lookUpVarStackAddr(func_name, getLeft()->getVarName())-4<<"(sp)"<<std::endl;
    }
};

class expr_right_bitshift : public expr_node {
    public: //_left is the expr we're incrementing, //right is the bitshfit amount
        expr_right_bitshift(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //g>>1 so bitshift g by 1 bit to the RIGHT.
        getLeft()->print(dst);
        dst<<">>";
        getRight()->print(dst);

    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {

        //only an operation so no need to store
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);

        auto func_name = Map.getCurrFunc();

        dst<<"srl "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;

        setVarName() = getLeft()->getVarName();
    }
};

class expr_assign : public expr_node {
    public:
        expr_assign(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        getLeft()->print(dst);
        dst<<"=";
        getRight()->print(dst);
        dst<<";"<<std::endl; //test this
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {        //executes instructions for LHS, RHS operands.
        
        int scope_index = Map.getCurrScopeIndex();
        //Assuming left is identifier:
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //Assuming right is constant:
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map); //li t0, int_literal


        if(reg_ctxt.isVarInReg(getLeft()->getVarName())) { //Checking if operands are int
            dst<<"add "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<"zero"<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            //stored RHS to LHS so now store back into stack and free LHS reg
            //so find register containing variable of varName LHS, and store in offset for the variable
            auto func_name = Map.getCurrFunc(); //string storing current func_name

            if(getLeft()->deref_flag == true) { //assumes dereference only occurs in LHS (NEED TO IMPLEMENT FOR RHS DEREF CASE)

                dst<<"sw "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", 0("<<reg_ctxt.findVarReg(getLeft()->deref_reg)<<")"<<std::endl;
                //Free reg:
                reg_ctxt.emptyReg(getLeft()->deref_reg);

            }
            else { //no deref operation

                dst<<"sw "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<Map.lookUpVarStackAddr(func_name, getLeft()->getName(), scope_index)-4<<"(sp)"<<std::endl;
                //stores register in stackPtr of LHS variable, doesnt work for dereferencing

            }
            reg_ctxt.emptyReg(getLeft()->getVarName()); //empties register that was used to store variable.
            //Also empty right reg, if its variable then it'll be called in next variable declaration:
            reg_ctxt.emptyReg(getRight()->getVarName());
            //setVarName() = getLeft()->getVarName();
            //not sure if above line is needed^, if needed then don't empty reg, needed, so don't empty reg. Only doing this because it frees more registers for use.
        }

        else if( freg_ctxt.isVarInReg(getLeft()->getVarName()) && freg_ctxt.VarType(getLeft()->getVarName())=="double" ) { //if operands are double
            dst<<"fmv.d "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            auto func_name = Map.getCurrFunc(); //string storing current func_name
            dst<<"fsd "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<Map.lookUpVarStackAddr(func_name, getLeft()->getName())-8<<"(sp)"<<std::endl;
            freg_ctxt.emptyReg(getLeft()->getVarName()); //empties register that was used to store variable.
            //Also empty right reg, if its variable then it'll be called in next variable declaration:
            freg_ctxt.emptyReg(getRight()->getVarName());
        }

        else if( freg_ctxt.isVarInReg(getLeft()->getVarName()) && freg_ctxt.VarType(getLeft()->getVarName())=="float" ) { //if operands are float
            dst<<"fmv.s "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<freg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
            auto func_name = Map.getCurrFunc(); //string storing current func_name
            dst<<"fsw "<<freg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<Map.lookUpVarStackAddr(func_name, getLeft()->getName())-4<<"(sp)"<<std::endl;
            freg_ctxt.emptyReg(getLeft()->getVarName()); //empties register that was used to store variable.
            //Also empty right reg, if its variable then it'll be called in next variable declaration:
            freg_ctxt.emptyReg(getRight()->getVarName());
        }

        else {
            dst<<"ERROR";
        }
    }
};

class expr_bitwise_AND : public expr_node {
        public:
        expr_bitwise_AND(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        getLeft()->print(dst);
        dst<<"&";
        getRight()->print(dst);
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {        //so: and a5, a4, a5 :
        //so: x & y, we getLeft()->getName() = variable name x
        //not sure if directly next expr has name property
        //assumes variables we're considering at stored in register.
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //Assuming right is constant:
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map); //li t0, int_literal
        //a = a&3; b = a&3;
        dst<<"and "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
        setVarName() = getLeft()->getVarName(); //so output of a + b is stored in a and that's assigned to varName so it can be called.
    }

};

class expr_bitwise_OR : public expr_node {
        public:
        expr_bitwise_OR(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        getLeft()->print(dst);
        dst<<"|";
        getRight()->print(dst);
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //Assuming right is constant:
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map); //li t0, int_literal
        //so: or a5, a4, a5 :
        //so: x | y, we getLeft()->getName() = variable name x
        //not sure if directly next expr has name property
        //assumes variables we're considering at stored in register.
        dst<<"or "<<reg_ctxt.findVarReg(getRight()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
        setVarName() = getRight()->getVarName(); //so output of a + b is stored in a and that's assigned to varName so it can be called.
    }
};

class expr_bitwise_XOR : public expr_node {
        public:
        expr_bitwise_XOR(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        getLeft()->print(dst);
        dst<<"^";
        getRight()->print(dst);
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {        //so: xor a5, a4, a5 :
        //so: x ^ y, we getLeft()->getName() = variable name x
        //not sure if directly next expr has name property
        //assumes variables we're considering at stored in register.
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //Assuming right is constant:
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map); //li t0, int_literal
        //a = a^3; b = a^3;
        dst<<"xor "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
        setVarName() = getLeft()->getVarName();
    }
};

class expr_is_equal : public expr_node {
        public:
        expr_is_equal(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        getLeft()->print(dst);
        dst<<"==";
        getRight()->print(dst);
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);

        //x == y
        //so: xor t0, x, y // t0 = x XOR y
        //sltu x0, t0, 1 // x0 = (t0 < 1) ? 1 : 0
        //so if x0 is 1 then x==y, x0 is 0 if x!=y

        std::string res1 = makeName("result");
        std::string res2 = makeName("result");
        std::string temp_reg1 = makeName("tempReg1");


        reg_ctxt.newVar(res1, IntType, "a0", "a7");
        reg_ctxt.newVar(res2, IntType, "a0", "a7");
        //sgt register with varName result, register with varName x, register with VarName y
        dst<<"xor "<<reg_ctxt.findVarReg(res1)<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
        dst<<"sltu "<<reg_ctxt.findVarReg(res2)<<", "<<reg_ctxt.findVarReg(res1)<<", 1"<<std::endl;
        reg_ctxt.emptyReg(res1);
        //so return reg_ctxt.findVarReg(res2)
        //dst<<"bnez "<<reg_ctxt.findVarReg(res2)<<", "; //label will be inputted by calling function or for loop, res2 or res1 for this?
        setVarName() = res2;
    }

};

class expr_less_than : public expr_node {
        public:
        expr_less_than(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        getLeft()->print(dst);
        dst<<"<";
        getRight()->print(dst);
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //x<y;
        //so: slt result, x, y; so assign result to new register;
        //Find new register in t0-t6 range and assigns it a variable of int type and name: result
        std::string res1 = makeName("result");
        reg_ctxt.newVar(res1, IntType, "a0", "a7");
        //slt register with varName result, register with varName x, register with VarName y
        //if x<y then findVarReg("result") is assigned 1, else 0
        //if bnez findVarReg("result"), label so if 1 then branch to label
        dst<<"slt "<<reg_ctxt.findVarReg(res1)<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
        reg_ctxt.emptyReg(getLeft()->getVarName());
        //dst<<"bnez "<<reg_ctxt.findVarReg(res1)<<", "; //label will be inputted by calling function or for loop
        setVarName() = res1;
    }

};

class expr_lt_equal : public expr_node {
        public:
        expr_lt_equal(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        getLeft()->print(dst);
        dst<<"<=";
        getRight()->print(dst);
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {        //implementation not working.
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //x<=y;
        //so: sle result, x, y; so assign result to new register;
        //Find new register in t0-t6 range and assigns it a variable of int type and name: result
        //maybe change name? so like unique name thingy in lab3, so result isn't searched again, instead we search a unique varname. Leave for now.
        std::string res1 = makeName("result");
        reg_ctxt.newVar(res1, IntType, "a0", "a7");

        //sle register with varName result, register with varName x, register with VarName y

        dst<<"sgt "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
        dst<<"xori "<<reg_ctxt.findVarReg(res1)<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", 1"<<std::endl;
        reg_ctxt.emptyReg(getLeft()->getVarName());
        //dst<<"sle "<<reg_ctxt.findVarReg(res1)<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
        //dst<<"bnez "<<reg_ctxt.findVarReg(res1)<<", "; //label will be inputted by calling function or for loop
        setVarName() = res1;
    }

};

class expr_greater_than : public expr_node {
        public:
        expr_greater_than(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        getLeft()->print(dst);
        dst<<">";
        getRight()->print(dst);
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //x>y;
        //so: sgt result, x, y; so assign result to new register;
        //Find new register in t0-t6 range and assigns it a variable of int type and name: result
        //maybe change name? so like unique name thingy in lab3, so result isn't searched again, instead we search a unique varname. Leave for now.
        std::string res1 = makeName("result");
        reg_ctxt.newVar(res1, IntType, "a0", "a7");
        //sgt register with varName result, register with varName x, register with VarName y
        dst<<"sgt "<<reg_ctxt.findVarReg(res1)<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
        reg_ctxt.emptyReg(getLeft()->getVarName());
        //dst<<"bnez "<<reg_ctxt.findVarReg(res1)<<", "; //label will be inputted by calling function or for loop
        setVarName() = res1;
    }

};

class expr_gt_equal : public expr_node {
        public:
        expr_gt_equal(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        getLeft()->print(dst);
        dst<<">=";
        getRight()->print(dst);
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //x>=y;
        //so: sge result, x, y; so assign result to new register;
        //Find new register in t0-t6 range and assigns it a variable of int type and name: result
        //maybe change name? so like unique name thingy in lab3, so result isn't searched again, instead we search a unique varname. Leave for now.
        std::string res1 = makeName("result");
        reg_ctxt.newVar(res1, IntType, "a0", "a7");
        //sgt register with varName result, register with varName x, register with VarName y
        dst<<"sge "<<reg_ctxt.findVarReg(res1)<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(getRight()->getVarName())<<std::endl;
        reg_ctxt.emptyReg(getLeft()->getVarName());
        //dst<<"bnez "<<reg_ctxt.findVarReg(res1)<<", "; //label will be inputted by calling function or for loop
        setVarName() = res1;
    }

};

class expr_logical_and : public expr_node {
        public:
        expr_logical_and(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        getLeft()->print(dst);
        dst<<"&&";
        getRight()->print(dst);
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //x && y
        //so: and t0, s1, s2; bnez t0, label. so if x&&y is true, t0 holds true value(1), so bnez t0, label so we branch to label as t0 holds 1 not zero.
        //Find new register in t0-t6 range and assigns it a variable of int type and name: result
        //maybe change name? so like unique name thingy in lab3, so result isn't searched again, instead we search a unique varname. Leave for now.
        std::string res1 = makeName("result");
        reg_ctxt.newVar(res1, IntType, "a0", "a7");

        std::string label_zero = makeName("label_zero");
        //sgt register with varName result, register with varName x, register with VarName y
        //try using xor, or
        std::string label_cont = makeName("label_continue");

        //beq left reg, zero, jump to label that loads zero in output reg
        dst<<"beq "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", zero"<<", "+label_zero<<std::endl;

        //if at this point then first value is 1 so check second value:

        dst<<"beq "<<reg_ctxt.findVarReg(getRight()->getVarName())<<", zero"<<", "+label_zero<<std::endl;

        //still jump to 0 label if second value is 0.
        //if we don't jump in both cases then must mean x&&y = true so:
        //storing result back in register storing variable a: //Wrong, storing in new register
        //findVarReg finds register variable name (res1) is stored in.
        dst<<"addi "<<reg_ctxt.findVarReg(res1)<<", "<<"zero"<<", 1"<<std::endl;

        //jump to end label:

        dst<<"beq zero, zero, "+label_cont<<std::endl;

        //label_zero:

        dst<<label_zero+": "<<std::endl;

        dst<<"addi "<<reg_ctxt.findVarReg(res1)<<", "<<"zero"<<", 0"<<std::endl;

        dst<<"beq zero, zero, "+label_cont<<std::endl;

        dst<<label_cont+": "<<std::endl;

        setVarName() = res1;
    }
};

class expr_logical_or : public expr_node {
        public:
        expr_logical_or(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        getLeft()->print(dst);
        dst<<"||";
        getRight()->print(dst);
    }
    //if left, right are variables then go into variable state, see registers stored at and xor these:
    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //x || y

        std::string res1 = makeName("result");
        reg_ctxt.newVar(res1, IntType, "a0", "a7");

        std::string label_one = makeName("label_one");
        std::string label_cont = makeName("label_continue");

        //beq(branch if equal) left reg, 1, jump to label that loads 1 in output reg
        //beq left, 1, label_one
        //check if there's beq using immediate value for comparison
        dst<<"bne "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", zero, "+label_one<<std::endl;

        //if at this point then first value is 0 so check second value:

        dst<<"bne "<<reg_ctxt.findVarReg(getRight()->getVarName())<<", zero, "+label_one<<std::endl;

        //if at this point then false false so store 0.
        //storing result back in register storing variable a: //Wrong, storing in new register
        //findVarReg finds register variable name (res1) is stored in.
        dst<<"addi "<<reg_ctxt.findVarReg(res1)<<", "<<"zero"<<", 0"<<std::endl;

        //jump to end label:

        dst<<"beq zero, zero, "+label_cont<<std::endl;

        //label_one:

        dst<<label_one+": "<<std::endl;

        dst<<"addi "<<reg_ctxt.findVarReg(res1)<<", "<<"zero"<<", 1"<<std::endl;

        dst<<"beq zero, zero, "+label_cont<<std::endl;

        dst<<label_cont+": "<<std::endl;

        setVarName() = res1;
    }

};

class expr_ident : public expr_node {
    public:
        expr_ident(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        dst<<getName();
    }
    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        //when declaring a variable, create variable state and use this to see what reg variable identifier is in.
        //what register is variable with identifier getName() in:

        //load from memory into register and set this register as setVarName(), setVarName() is the name of the variable in the register we're searching for.
        //load into register regName given offset

        //lw findFreeReg varName offset

        //Check type to decide whether to load in normal reg or freg:

        //Current func_name:
        auto func_name = Map.getCurrFunc();
        int scope_index = Map.getCurrScopeIndex();
        //Finding free reg:

        //expr_ident can also be pntr type so need to check:
        if(Map.lookUpVarIsPntr(func_name, getName())==true, scope_index) {
        //output an add specifier or multiple that the expr_add receives and adds accordingly

            if(Map.lookUpVarType(func_name, getName(), scope_index)==IntType) {
                add_specifier = 4;
            }

            if(Map.lookUpVarTypePrint(func_name, getName(), scope_index)=="int") { //so int *pntr; can add for different types double *pntr, char *pntr

                reg_ctxt.newVar(getName(), Map.lookUpVarType(func_name, getName(), scope_index) , "a0", "a7", true); //freeReg found and stored in it variable of name getName()

                dst<<"lw "<<reg_ctxt.findVarReg(getName())<<", "<<Map.lookUpVarStackAddr(func_name, getName(), scope_index)-4<<"(sp)"<<std::endl;

                setVarName() = getName(); //assigns VarName as the identifier name: getName();

            }

            //Need to add flag if pntr or can just check Left.

            //copy int stuff for pntr type and make the rest else if

        }
        else if(Map.lookUpVarTypePrint(func_name, getName(), scope_index)=="int") {

            reg_ctxt.newVar(getName(), Map.lookUpVarType(func_name, getName(), scope_index) , "a0", "a7"); //freeReg found and stored in it variable of name getName()

            dst<<"lw "<<reg_ctxt.findVarReg(getName())<<", "<<Map.lookUpVarStackAddr(func_name, getName(), scope_index)-4<<"(sp)"<<std::endl;

            setVarName() = getName(); //assigns VarName as the identifier name: getName();
        }
        else if(Map.lookUpVarTypePrint(func_name, getName())=="double") {
            freg_ctxt.newVar(getName(), Map.lookUpVarType(func_name, getName()) , "fa0", "fa7"); //freeReg found and stored in it variable of name getName()
            dst<<"fld "<<freg_ctxt.findVarReg(getName())<<", "<<Map.lookUpVarStackAddr(func_name, getName())-8<<"(sp)"<<std::endl;
            setVarName() = getName();
        }
        else if(Map.lookUpVarTypePrint(func_name, getName())=="float") {
            freg_ctxt.newVar(getName(), Map.lookUpVarType(func_name, getName()) , "fa0", "fa7"); //freeReg found and stored in it variable of name getName()
            dst<<"flw "<<freg_ctxt.findVarReg(getName())<<", "<<Map.lookUpVarStackAddr(func_name, getName())-4<<"(sp)"<<std::endl;
            setVarName() = getName();
        }
        else {
            reg_ctxt.newVar(getName(), Map.lookUpVarType(func_name, getName(), scope_index) , "a0", "a7"); //freeReg found and stored in it variable of name getName()
            dst<<"lw "<<reg_ctxt.findVarReg(getName())<<", "<<Map.lookUpVarStackAddr(func_name, getName(), scope_index)-4<<"(sp)"<<std::endl;
            setVarName() = getName(); //assigns VarName as the identifier name: getName();
        }
    }

};

class expr_func_call : public expr_node {
    private:
        param_node* param;
    public:
        expr_func_call(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val, param_node* _param) :
        expr_node(_left, _right, _name, _int_val, _string_val), param(_param)
        {is_func_call = true;} //assign func_call flag to true.

    virtual void print(std::ostream& dst) override {
        //output: x (variable identifier)
        dst<<getName()<<"(";
        if(param!=NULL) {param->print(dst);}
        dst<<")";
    }

    virtual void stackBuild(stackAST& Map, std::vector<variable_state>& vec) {
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) { //store params in certain registers
        //store current ra somewhere (in a register), then do call and then assign ra
        auto func_name = Map.getCurrFunc();

        //std::string temp_ra = makeName("temp_ra");
        //reg_ctxt.newVar(temp_ra, IntType, "t0", "t6"); //need to figure out whether s0, s11 or t0, t6. Because might exceed t0, t6 range and then we're reassigning already assigned registers. Should be sorted once done with stack.
        //dst<<"add "<<reg_ctxt.findVarReg(temp_ra)<<", "<<"zero"<<", ra"<<std::endl; //add temp_ra, zero, ra

        //store temp reg values in mem:
        //Assumes return register is only a0.
        //dst<<"sw a0, "<<Map.lookUpVarStackAddr(func_name, "rega0")-4<<"(sp)"<<std::endl;
        dst<<"sw a1, "<<Map.lookUpVarStackAddr(func_name, "rega1")-4<<"(sp)"<<std::endl;
        dst<<"sw a2, "<<Map.lookUpVarStackAddr(func_name, "rega2")-4<<"(sp)"<<std::endl;
        dst<<"sw a3, "<<Map.lookUpVarStackAddr(func_name, "rega3")-4<<"(sp)"<<std::endl;
        dst<<"sw a4, "<<Map.lookUpVarStackAddr(func_name, "rega4")-4<<"(sp)"<<std::endl;
        dst<<"sw a5, "<<Map.lookUpVarStackAddr(func_name, "rega5")-4<<"(sp)"<<std::endl;
        dst<<"sw a6, "<<Map.lookUpVarStackAddr(func_name, "rega6")-4<<"(sp)"<<std::endl;
        dst<<"sw a7, "<<Map.lookUpVarStackAddr(func_name, "rega7")-4<<"(sp)"<<std::endl;

        //if arguments, then need to store them in a0-a6 before jump so: and put this code before beq...
        store_flag = false; //must load params into regs not store
        if(param!=NULL) {param->riscv(dst, reg_ctxt, freg_ctxt, Map, store_flag, 0, 0, noResetRegs);}

        //dst<<"beq zero, zero, "<<getName()<<std::endl;
        dst<<"call "<<getName()<<std::endl;
        //load ra from mem
        dst<<"lw ra, "<<Map.lookUpVarStackAddr(func_name, "tempStackMem")-4<<"(sp)"<<std::endl;

        //load temp reg values:

        //dst<<"lw a0, "<<Map.lookUpVarStackAddr(func_name, "rega0")-4<<"(sp)"<<std::endl;
        dst<<"lw a1, "<<Map.lookUpVarStackAddr(func_name, "rega1")-4<<"(sp)"<<std::endl;
        dst<<"lw a2, "<<Map.lookUpVarStackAddr(func_name, "rega2")-4<<"(sp)"<<std::endl;
        dst<<"lw a3, "<<Map.lookUpVarStackAddr(func_name, "rega3")-4<<"(sp)"<<std::endl;
        dst<<"lw a4, "<<Map.lookUpVarStackAddr(func_name, "rega4")-4<<"(sp)"<<std::endl;
        dst<<"lw a5, "<<Map.lookUpVarStackAddr(func_name, "rega5")-4<<"(sp)"<<std::endl;
        dst<<"lw a6, "<<Map.lookUpVarStackAddr(func_name, "rega6")-4<<"(sp)"<<std::endl;
        dst<<"lw a7, "<<Map.lookUpVarStackAddr(func_name, "rega7")-4<<"(sp)"<<std::endl;

        //setVarName() = "a0"; //getVarName() is empty currently
        //we've returned from function call, now we must store a0 that contains return into another free reg and then output this reg from func_call
        std::string arg_reg = makeName("arg_reg"); //temporary argument register to store output of func_call
        reg_ctxt.newVar(arg_reg, IntType, "a0", "a7");
        //mv arg_reg a0, //make sure not overrwrting a0
        dst<<"mv "<<reg_ctxt.findVarReg(arg_reg)<<", a0"<<std::endl;
        setVarName() = arg_reg;

    }

    virtual ~expr_func_call() {
        delete param;
    }
};

class expr_addr_ident : public expr_node {
    public: //_name is identifier
        expr_addr_ident(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: 5 (int literal)
        dst<<"&"<<getName();
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {

        //Find identifier and load its stack offset into a register:
        auto func_name = Map.getCurrFunc();

        //store offset of identifier onto a register
        std::string addr_of_ident = makeName("addr_of_ident");

        //register variable will store address of so will be pointer, identifier type
        reg_ctxt.newVar(addr_of_ident, Map.lookUpVarType(func_name, getName()), "t0", "t6", true); //either put in t0-t6 or a0-a7

        //Load offset into register:
        dst<<"addi "<<reg_ctxt.findVarReg(addr_of_ident)<<", sp, "<<Map.lookUpVarStackAddr(func_name, getName())-4<<std::endl;

        setVarName() = addr_of_ident;
    }
};

class expr_deref_ident : public expr_node {
    public: //_name is identifier
        expr_deref_ident(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        dst<<"*"<<getName();
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        //Doing dereference so:
        deref_flag = true;

        //Find identifier and load its contents into a register
        auto func_name = Map.getCurrFunc();

        //store contents of stack offset into a register
        std::string y_contents = makeName("y_contents");

        //register variable will store contents of a stack offset, the stack offset is the variable we're dereferencing
        reg_ctxt.newVar(y_contents, Map.lookUpVarType(func_name, getName()), "t0", "t6", false); //either put in t0-t6 or a0-a7, false as we're dereferencing so just int

        //lw a0, y-offset
        dst<<"lw "<<reg_ctxt.findVarReg(y_contents)<<", "<<Map.lookUpVarStackAddr(func_name, getName())-4<<"(sp)"<<std::endl;

        //a0 contains x address/offset so loading this into a new reg gives value at x
        std::string x_var = makeName("x_var");
        reg_ctxt.newVar(x_var, Map.lookUpVarType(func_name, getName()), "t0", "t6", false); //same type as pointer as then only can address of x be assigned to y.

        //so x_var register now contains x
        dst<<"lw "<<reg_ctxt.findVarReg(x_var)<<", 0("<<reg_ctxt.findVarReg(y_contents)<<")"<<std::endl;

        //Free y_content reg:
        //reg_ctxt.emptyReg(y_contents);
        deref_reg = y_contents;
        setVarName() = x_var;

        //assign uses getLeft()->getName() to store the address, so maybe have deref flag that we set to 1 and if 1 then we use getLeft()->assignAdr()
        //contents of reg storing y_contents is stack addr of x so that what we want.
        //maybe load x address into register like a0 storing x address and don't empty reg and send that reg to expr_assign and use that to store so: 0(a0)

    }
};

class expr_int_literal : public expr_node {
    public:
        expr_int_literal(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: 5 (int literal)
        dst<<get_int_val();
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        //std::string res1 = std::to_string(get_int_val());
        std::string res2 = makeName("int_val");
        reg_ctxt.newVar(res2, IntType, "t0", "t6"); //need to figure out whether s0, s11 or t0, t6. Because might exceed t0, t6 range and then we're reassigning already assigned registers. Should be sorted once done with stack.
        dst<<"li "<<reg_ctxt.findVarReg(res2)<<", "<<get_int_val()<<std::endl;
        setVarName() = res2; //now this reassigns this objects varName property and can be called by any class using getVarName() which gets variable name which we can use to search which register it is in.
    }
};

class expr_char_literal : public expr_node {
    public: //string_val is used to store char literal
        expr_char_literal(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: "dog" (string literal)
        //dst<<" \" ";
        dst<<get_string_val();
        //dst<<" \" "; //might be problem with these quotation marks
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {

        //We want to load ASCII equivalent of char in register and output register:
        std::string charLiteral = makeName("charLiteral");
        reg_ctxt.newVar(charLiteral, CharType, "t0", "t6"); //not a variable so store in temp reg
        //char is given by parser as "'char_literal'", so 1st index is the actual char.
        dst<<"li "<<reg_ctxt.findVarReg(charLiteral)<<", "<<int(get_string_val()[1])<<std::endl;
        setVarName() = charLiteral;
    }

};

class expr_string_literal : public expr_node {
    public:
        expr_string_literal(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

    virtual void print(std::ostream& dst) override {
        //output: "dog" (string literal)
        //dst<<" \" ";
        dst<<get_string_val();
        //dst<<" \" "; //might be problem with these quotation marks
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {}

};

class expr_double_literal : public expr_node {
    private:
        double double_val;
    public:
     //can remove str_double_val and use string_val instead
        expr_double_literal(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val, std::string _str_double_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {double_val = std::stod(_str_double_val);}

    double get_double_val() {return double_val;}
    double* get_double_val_pntr() {return &double_val;}

    virtual void print(std::ostream& dst) override {
        //output: 5 (int literal)
        dst<<get_double_val();
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        //To load a double immediate into a double register, need to use reinterpret case to convert double to 64 bit integer and then split accordingly
        double* double_pntr = get_double_val_pntr();
        uint64_t* bits = reinterpret_cast<uint64_t*>(double_pntr); //so memory storing double_val is now interpreted as a 64 bit int.
        uint32_t upper = *bits >> 32; //get upper 32 binary bits by left shifting by 32 places.
        uint32_t lower = *bits & 0xFFFFFFFF; //has the effect of removing upper 32 bits

        //Loading these values in normal registers: as we can only do conversion of normal register to double reg:
        std::string temp_reg1 = makeName("int_val");
        reg_ctxt.newVar(temp_reg1, IntType, "t0", "t6"); //need to figure out whether s0, s11 or t0, t6. Because might exceed t0, t6 range and then we're reassigning already assigned registers. Should be sorted once done with stack.
        dst<<"li "<<reg_ctxt.findVarReg(temp_reg1)<<", "<<upper<<std::endl;

        std::string temp_reg2 = makeName("int_val");
        reg_ctxt.newVar(temp_reg2, IntType, "t0", "t6"); //need to figure out whether s0, s11 or t0, t6. Because might exceed t0, t6 range and then we're reassigning already assigned registers. Should be sorted once done with stack.
        dst<<"li "<<reg_ctxt.findVarReg(temp_reg2)<<", "<<lower<<std::endl;

        //Convert to double register:
        std::string fval_name = makeName("fval_name");
        freg_ctxt.newVar(fval_name, DoubleType, "ft0", "ft7");
        //might have to be fcvt.d.w or something
        dst<<"fmv.d.x "<<freg_ctxt.findVarReg(fval_name)<<", "<<reg_ctxt.findVarReg(temp_reg1)<<std::endl;
        setVarName() = fval_name;

        //free a_regs used:
        reg_ctxt.emptyReg(temp_reg1);
        reg_ctxt.emptyReg(temp_reg2);
    }
};

class expr_float_literal : public expr_node {
    private:
        float float_val;
    public:
     //can remove str_double_val and use string_val instead
        expr_float_literal(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val, std::string _str_float_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {float_val = std::stof(_str_float_val);}

    float get_float_val() {return float_val;}
    float* get_float_val_pntr() {return &float_val;}

    virtual void print(std::ostream& dst) override {
        //output: 5 (int literal)
        dst<<get_float_val();
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        //To load a double immediate into a double register, need to use reinterpret case to convert double to 64 bit integer and then split accordingly
        float* float_pntr = get_float_val_pntr();
        uint32_t* bits = reinterpret_cast<uint32_t*>(float_pntr); //so memory storing double_val is now interpreted as a 64 bit int.

        //Loading these values in normal registers: as we can only do conversion of normal register to double reg:
        std::string temp_reg1 = makeName("int_val");
        reg_ctxt.newVar(temp_reg1, IntType, "t0", "t6"); //need to figure out whether s0, s11 or t0, t6. Because might exceed t0, t6 range and then we're reassigning already assigned registers. Should be sorted once done with stack.
        dst<<"li "<<reg_ctxt.findVarReg(temp_reg1)<<", "<<*bits<<std::endl;

        //Convert to float register:
        std::string fval_name = makeName("fval_name");
        freg_ctxt.newVar(fval_name, FloatType, "ft0", "ft7");
        //might have to be fcvt.d.w or something
        dst<<"fmv.w.x "<<freg_ctxt.findVarReg(fval_name)<<", "<<reg_ctxt.findVarReg(temp_reg1)<<std::endl;
        setVarName() = fval_name;

        //free a_regs used:
        reg_ctxt.emptyReg(temp_reg1);
    }
};

class expr_array : public expr_node{
    public:
        expr_array(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val) :
        expr_node(_left, _right, _name, _int_val, _string_val)
        {}

        virtual void print(std::ostream& dst) override {
            // this is x[index] = value.
            dst<<getName();
            dst<<"[";
            getLeft()->print(dst);
            dst<<"]";
            if(getRight()!=NULL){
                dst<<"=";
                getRight()->print(dst);
                dst<<";"<<std::endl;
            }
        }

        virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
            //ADD CHECK TO SEE IF PNTR OR ARRAY TYPE, IF PNTR THEN DO BELOW FOR INDEXING:

            //Checking if identifier is pntr type, so must check if its previously mentioned in map?
            auto func_name = Map.getCurrFunc();

            if(Map.lookUpVarIsPntr(func_name, getName())){
                reg_ctxt.newVar(getName(), Map.lookUpVarType(func_name, getName()), "a0", "a7", true); //so calling p pntr into a register

                //loading p into register
                dst<<"lw "<<reg_ctxt.findVarReg(getName())<<", "<<Map.lookUpVarStackAddr(func_name, getName())-4<<"(sp)"<<std::endl;

                //adding to p based on index and size of p type:

                //need to make add_specifier custom based on p type, for now just use 4
                if(Map.lookUpVarTypePrint(func_name, getName())=="int") {
                    add_specifier = 4;
                }
                else if (Map.lookUpVarTypePrint(func_name, getName())=="char") {
                    add_specifier = 1;
                }

                std::string add_spec_reg = makeName("add_spec");
                reg_ctxt.newVar(add_spec_reg, IntType, "t0", "t6");

                //not sure if this add_spec_reg is 0 - actually its solved now
                dst<<"addi "<<reg_ctxt.findVarReg(add_spec_reg)<<", zero, "<<add_specifier<<std::endl;

                //loading index into a register:
                getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map); //outputs register that stores index as immediate value

                //multiply specifier by index and store in add_spec_reg (not sure about storin gback in add_spec_reg)
                dst<<"mul "<<reg_ctxt.findVarReg(add_spec_reg)<<", "<<reg_ctxt.findVarReg(getLeft()->getVarName())<<", "<<reg_ctxt.findVarReg(add_spec_reg)<<std::endl;

                //Adding to reg storing address

                dst<<"add "<<reg_ctxt.findVarReg(getName())<<", "<<reg_ctxt.findVarReg(getName())<<", "<<reg_ctxt.findVarReg(add_spec_reg)<<std::endl;
                setVarName() = getName(); //so output of a + b is stored in a and that's assigned to varName so it can be called.
                //Empty RHS reg as sum is stored in LHS reg.
                //need to empty reg:
                reg_ctxt.emptyReg(add_spec_reg);

                deref_flag = true;

                //a0 contains x address/offset so loading this into a new reg gives value at x
                std::string x_var = makeName("x_var");
                reg_ctxt.newVar(x_var, Map.lookUpVarType(func_name, getName()), "t0", "t6", false); //same type as pointer as then only can address of x be assigned to y.

                //so x_var register now contains x
                dst<<"lw "<<reg_ctxt.findVarReg(x_var)<<", 0("<<reg_ctxt.findVarReg(getName())<<")"<<std::endl;

                //Free y_content reg:
                //reg_ctxt.emptyReg(y_contents);
                deref_reg = getName();
                setVarName() = x_var;
            }
            else{
                // so its an array and we want to assign the value given by the index to the value provided.

                // the basic operation involves finding the variable in the stack, and then assigning like 4 or 8 * the index to the variable.
                if(Map.lookUpVarTypePrint(func_name, getName()) == "int"){
                    // means you have to increment by 4 an duse regular registers

                    //could do getLeft()->riscv(...)
                    //then use reg_ctxt.findVarReg(getLeft()->getVarName())

                    int head = Map.lookUpVarStackAddr(func_name, getName()); // this defines the head of the stack where the array is stored, all offsets are calculated from here

                    int offset = 4 * getLeft()->get_int_val(); // this defines what the offset is and where we are storing the variable.

                    int position = head - offset; //check if this is correct

                    std::string arr_offset = makeName("offset"); //stores index
                    reg_ctxt.newVar(arr_offset, IntType, "t0", "t6");

                    std::string multiply_imm = makeName("multiply_imm");
                    reg_ctxt.newVar(multiply_imm, IntType, "t0", "t6");

                    getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map); // loads index into a register
                    dst<<"mv "<<reg_ctxt.findVarReg(arr_offset)<<","<<reg_ctxt.findVarReg(getLeft()->getVarName())<<std::endl; // loads the offset into a register 

                    //CHANGE BACK TO +4;
                    dst<<"li "<<reg_ctxt.findVarReg(multiply_imm)<<", 4"<<std::endl; // loads the offset into a register

                    dst<<"mul "<<reg_ctxt.findVarReg(arr_offset)<<", "<<reg_ctxt.findVarReg(arr_offset)<<", "<<reg_ctxt.findVarReg(multiply_imm)<<std::endl; // used for the offset register
                    //so arr_offset stores index as immediate, we multiply index by type offset (so x 4 for int)
                    
                    reg_ctxt.emptyReg(multiply_imm);
                    
                    std::string arr_position = makeName("position");
                    reg_ctxt.newVar(arr_position, IntType, "t0", "t6");

                    dst<<"li "<<reg_ctxt.findVarReg(arr_position)<<","<<Map.lookUpVarStackAddr(func_name, getName())<<std::endl; //loads address of head into reg
                    dst<<"sub "<<reg_ctxt.findVarReg(arr_position)<<","<<reg_ctxt.findVarReg(arr_offset)<<","<<reg_ctxt.findVarReg(arr_position)<<std::endl; //find new position by subtracting index * type size to get new position(address)
                    // no need for the offset anymore

                    std::string array_var = makeName("array_var");
                    reg_ctxt.newVar(array_var, IntType, "t0", "t6");
                    //arr_position contains the address we want to write to

                    if(getRight()!=NULL){
                        //dst<<"li "<<reg_ctxt.findVarReg(array_var)<<","<<getRight()->get_int_val()<<std::endl;
                        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);
                        dst<<"add "<<reg_ctxt.findVarReg(arr_position)<<", "<<reg_ctxt.findVarReg(arr_position)<<", sp"<<std::endl;                        
                        dst<<"sw "<<reg_ctxt.findVarReg(getRight()->getVarName())<<", 0("<<reg_ctxt.findVarReg(arr_position)<<")"<<std::endl;
                        reg_ctxt.emptyReg(array_var);
                        reg_ctxt.emptyReg(getRight()->getVarName());
                    }
                    else { // means you want to use this is an expression, so like x + y[0] or something.
                        std::string array_return = makeName("array_return");
                        reg_ctxt.newVar(array_return, IntType, "a0", "a7");
                        //CHANGE BACK TO ADD:
                        dst<<"add "<<reg_ctxt.findVarReg(arr_position)<<", "<<reg_ctxt.findVarReg(arr_position)<<", sp"<<std::endl;
                        dst<<"lw "<<reg_ctxt.findVarReg(array_return)<<", 0("<<reg_ctxt.findVarReg(arr_position)<<")"<<std::endl;
                        setVarName() = array_return;
                    }
                    reg_ctxt.emptyReg(arr_offset); // empty register not working?
                    reg_ctxt.emptyReg(arr_position); // I have used the same register for all load and store operations
                    reg_ctxt.emptyReg(getLeft()->getVarName());
                }
                else if (Map.lookUpVarTypePrint(func_name, getName()) == "double"){
                    // means you have to increment by 8 an use float registers
                }
                else if(Map.lookUpVarTypePrint(func_name, getName()) == "float"){
                    // means you have to increment by 4 an use float registers
                }
                else if(Map.lookUpVarTypePrint(func_name, getName()) == "char"){
                    // means you have to increment by 1

                     // means you have to increment by 4 an duse regular registers

                    //could do getLeft()->riscv(...)
                    //then use reg_ctxt.findVarReg(getLeft()->getVarName())

                    int head = Map.lookUpVarStackAddr(func_name, getName()); // this defines the head of the stack where the array is stored, all offsets are calculated from here

                    int offset = 4 * getLeft()->get_int_val(); // this defines what the offset is and where we are storing the variable.

                    int position = head - offset; //check if this is correct

                    std::string arr_offset = makeName("offset"); //stores index
                    reg_ctxt.newVar(arr_offset, IntType, "t0", "t6");

                    std::string multiply_imm = makeName("multiply_imm");
                    reg_ctxt.newVar(multiply_imm, IntType, "t0", "t6");

                    getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map); // loads index into a register
                    dst<<"mv "<<reg_ctxt.findVarReg(arr_offset)<<","<<reg_ctxt.findVarReg(getLeft()->getVarName())<<std::endl; // loads the offset into a register 

                    //CHANGE BACK TO +4;
                    dst<<"li "<<reg_ctxt.findVarReg(multiply_imm)<<", 4"<<std::endl; // loads the offset into a register

                    dst<<"mul "<<reg_ctxt.findVarReg(arr_offset)<<", "<<reg_ctxt.findVarReg(arr_offset)<<", "<<reg_ctxt.findVarReg(multiply_imm)<<std::endl; // used for the offset register
                    //so arr_offset stores index as immediate, we multiply index by type offset (so x 4 for int)
                    
                    reg_ctxt.emptyReg(multiply_imm);
                    
                    std::string arr_position = makeName("position");
                    reg_ctxt.newVar(arr_position, IntType, "t0", "t6");

                    dst<<"li "<<reg_ctxt.findVarReg(arr_position)<<","<<Map.lookUpVarStackAddr(func_name, getName())<<std::endl; //loads address of head into reg
                    dst<<"add "<<reg_ctxt.findVarReg(arr_position)<<","<<reg_ctxt.findVarReg(arr_offset)<<","<<reg_ctxt.findVarReg(arr_position)<<std::endl; //find new position by subtracting index * type size to get new position(address)
                    dst<<"addi "<<reg_ctxt.findVarReg(arr_position)<<","<<reg_ctxt.findVarReg(arr_position)<<", -4"<<std::endl;
                    // no need for the offset anymore

                    std::string array_var = makeName("array_var");
                    reg_ctxt.newVar(array_var, IntType, "t0", "t6");
                    //arr_position contains the address we want to write to

                    if(getRight()!=NULL){
                        //dst<<"li "<<reg_ctxt.findVarReg(array_var)<<","<<getRight()->get_int_val()<<std::endl;
                        getRight()->riscv(dst, reg_ctxt, freg_ctxt, Map);
                        dst<<"add "<<reg_ctxt.findVarReg(arr_position)<<", "<<reg_ctxt.findVarReg(arr_position)<<", sp"<<std::endl;                        
                        dst<<"sw "<<reg_ctxt.findVarReg(getRight()->getVarName())<<", 0("<<reg_ctxt.findVarReg(arr_position)<<")"<<std::endl;
                        reg_ctxt.emptyReg(array_var);
                        reg_ctxt.emptyReg(getRight()->getVarName());
                    }
                    else { // means you want to use this is an expression, so like x + y[0] or something.
                        std::string array_return = makeName("array_return");
                        reg_ctxt.newVar(array_return, IntType, "a0", "a7");
                        //CHANGE BACK TO ADD:
                        dst<<"add "<<reg_ctxt.findVarReg(arr_position)<<", "<<reg_ctxt.findVarReg(arr_position)<<", sp"<<std::endl;
                        dst<<"lw "<<reg_ctxt.findVarReg(array_return)<<", 0("<<reg_ctxt.findVarReg(arr_position)<<")"<<std::endl;
                        setVarName() = array_return;
                    }
                    reg_ctxt.emptyReg(arr_offset); // empty register not working?
                    reg_ctxt.emptyReg(arr_position); // I have used the same register for all load and store operations
                    reg_ctxt.emptyReg(getLeft()->getVarName());
                }
                else{
                    // just a failsafe else
                }
            }
        }
};

class expr_sizeof : public expr_node{
    private:
        type_node* type;
    public:
        expr_sizeof(expr_node* _left, expr_node* _right, std::string _name, int _int_val, std::string _string_val, type_node* _type) :
        expr_node(_left, _right, _name, _int_val, _string_val), type(_type)
        {}

        type_node* getType(){
            return type;
        }

        //_left = can just assign this to be the variable. For example, sizeof(x). Needs to be an expr because sizeof(x+1) is valid, could be null too
        //_right = this isn't used i think
        //_name = ""
        //_int_val = 0
        //_string_val = 0
        //_type = new, optional type of variable - sizeof(int) if NULL, do nothing here.

        virtual void print(std::ostream& dst) override {
            if(getLeft() != NULL){
                // means you want sizeof(expr) so sizeof(x+1) or something
                dst<<"sizeof(";
                getLeft()->print(dst);
                dst<<")";
            } else if (getType()!=NULL){
                // means you want sizeof(type) so sizeof(int) or something
                dst<<"sizeof(";
                getType()->print(dst);
                dst<<")";
            }
        }

        virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
            int size_var = 0;
            std::string store = makeName("size_of");
            varType var;
            if(getType()!=NULL){
                var = getType()->get_kind();
                // means you want sizeof(type) so sizeof(int) or something
            } else if (getLeft()!=NULL){
                getLeft()->riscv(dst, reg_ctxt, freg_ctxt, Map);
                var = reg_ctxt.getVarType(getLeft()->getName()).getType();
            }
            if(var==DoubleType){
                size_var = 8;
            }
            else if(var==ShortType){
                size_var = 2;
            }
            else if(var==VoidType){
                size_var = 1;
            }
            else if(var==CharType){
                size_var = 1;
            }
            else{
                size_var = 4;
            }
            reg_ctxt.newVar(store, var, "a0", "a7");
            dst<<"li "<<reg_ctxt.findVarReg(store)<<", "<<size_var<<std::endl;
            setVarName() = store;
        }

};

#endif
