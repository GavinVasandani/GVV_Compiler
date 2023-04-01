#ifndef ast_stmnt_node_hpp
#define ast_stmnt_node_hpp
//make true_expr a public variable in ast_node, so in all constructors add it to {}
#include <iostream>
#include <string>
#include <queue>
#include <vector>

class case_stmnt;

typedef std::vector<case_stmnt*> CaseList;
typedef CaseList* CaseListPtr;

class stmnt_node : public node { //create child node for each type of stmnt_node: loop, conditional, block, decl, return, expr
    private: //some are removed from struct stmnt_node as they're needed specifically in certain child classes and not in others
        expr_node* start_expr; //starting value in for loop
        expr_node* true_expr; //true condition in for loop
        expr_node* incr_expr; //incrementing behaviour in for loop
        stmnt_node* body; //set of statements that execute in if/for loop
    protected: //constructor
        stmnt_node(expr_node* _start_expr, expr_node* _true_expr, expr_node* _incr_expr, stmnt_node* _body) :
        start_expr(_start_expr),
        true_expr(_true_expr),
        incr_expr(_incr_expr),
        body(_body)
        {}

    public:
        //get functions for child/derived classes to access member variables in base class:
        expr_node* get_start_expr() {return start_expr;}
        expr_node* get_true_expr() {return true_expr;}
        expr_node* get_incr_expr() {return incr_expr;}
        stmnt_node* get_body() {return body;}
        //no need for next (actually not sure maybe needed)

    //Destructor using RAII:
        virtual ~stmnt_node() {
            delete start_expr;
            delete true_expr;
            delete incr_expr;
            delete body;
        }
}; //still abstract class as void print hasn't be overridden.

//for child class: for (i=0; i<10; i++) { a = a+1;}
class for_stmnt : public stmnt_node {
    private:
    decl_node* func_scope;
    public: //maybe add null defaults for each input param so we don't need to explicitly put null for every input param when calling constructor
        for_stmnt(expr_node* _start_expr, expr_node* _true_expr, expr_node* _incr_expr, stmnt_node* _body, decl_node* _func_scope) :
        stmnt_node(_start_expr, _true_expr, _incr_expr, _body), func_scope(_func_scope)
        {}

    virtual void print(std::ostream& dst) override{
        //expected output: for (i=0; i<10; i++) { a = a+1;}
        dst<<"for (";
        //to prevent seg fault:
        if(get_start_expr()) {get_start_expr()->print(dst);}
        dst<<";";
        if(get_true_expr()) {get_true_expr()->print(dst);}
        dst<<";";
        if(get_incr_expr()) {get_incr_expr()->print(dst);}
        dst<<")";

        func_scope->print(dst);

        //dst<<"}"<<std::endl; //this allows us to use same for_stmnt class in: for (i=0; i++) {} just assign true_expr input param as NULL
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        //initialize start_expr so outputs its RISCV code:
        get_start_expr()->riscv(dst, reg_ctxt, freg_ctxt, Map); //this outputs the for loops start code.
        //label for loop condition:
        std::string loop = makeName("loop");
        std::string cont = makeName("cont");
        std::string end = makeName("end");

        dst<<loop+": "<<std::endl; //need to make these loop labels unique like in lab3, set as a string variable
        get_true_expr()->riscv(dst, reg_ctxt, freg_ctxt, Map); //this outputs the for loops condition code.
        std::string var_name = get_true_expr()->getVarName(); //varName that is used to search for register storing slt condition output
        dst<<"bnez "<<reg_ctxt.findVarReg(var_name)<<", "+cont<<std::endl;

        //jump to end label if for loop condition is false
        dst<<"j "+end<<std::endl;

        //jump to cont label if for loop condition is true
        dst<<cont+": "<<std::endl; //along same line
        func_scope->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //increment counter:
        get_incr_expr()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        //jump back to loop condition:
        dst<<"j "+loop<<std::endl;

        //Code below this is part of end label
        dst<<end+": "<<std::endl;
        //dst<<"exit: "<<std::endl; //exit label
    }
};

class while_stmnt : public stmnt_node {
    private:
    decl_node* func_scope;
    public: //maybe add null defaults for each input param so we don't need to explicitly put null for every input param when calling constructor
        while_stmnt(expr_node* _start_expr, expr_node* _true_expr, expr_node* _incr_expr, stmnt_node* _body, decl_node* _func_scope) :
        stmnt_node(_start_expr, _true_expr, _incr_expr, _body), func_scope(_func_scope)
        {}

    virtual void print(std::ostream& dst) override{
        //expected output: for (i=0; i<10; i++) { a = a+1;}
        dst<<"while (";
        //to prevent seg fault:
        if(get_true_expr()) {get_true_expr()->print(dst);}
        dst<<")";
        func_scope->print(dst);

        //dst<<"}"<<std::endl; //this allows us to use same for_stmnt class in: for (i=0; i++) {} just assign true_expr input param as NULL
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        //label for loop condition:
        std::string loop = makeName("loop");
        std::string end = makeName("end");

        dst<<"j "<<loop<<std::endl;

        dst<<loop+": "<<std::endl; //need to make these loop labels unique like in lab3, set as a string variable
        get_true_expr()->riscv(dst, reg_ctxt, freg_ctxt, Map); //this outputs the for loops condition code.

        std::string var_name = get_true_expr()->getVarName(); //varName that is used to search for register storing slt condition output
        dst<<"beqz "<<reg_ctxt.findVarReg(var_name)<<", "+end<<std::endl;

        func_scope->riscv(dst, reg_ctxt, freg_ctxt, Map);

        dst<<"j "+loop<<std::endl;

        dst<<end+": "<<std::endl;
        //continuation of rest of program code.
    }
};

//if child class: if (a<10) { a = a+1;}
class if_stmnt : public stmnt_node {
    private:
    decl_node* func_scope;
    public:
        if_stmnt(expr_node* _start_expr, expr_node* _true_expr, expr_node* _incr_expr, stmnt_node* _body, decl_node* _func_scope) :
        stmnt_node(_start_expr, _true_expr, _incr_expr, _body), func_scope(_func_scope)
        {}
    virtual void print(std::ostream& dst) override{
        //expected output: if (a<10) { a = a+1;}
        dst<<"if (";
        if(get_true_expr()) {get_true_expr()->print(dst);}
        dst<<")";
        func_scope->print(dst);
        //dst<<"}"<<std::endl;
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        //label for else condition:
        std::string elseLabel = makeName("else");

        get_true_expr()->riscv(dst, reg_ctxt, freg_ctxt, Map); //this outputs the if stmnt condition.
        std::string var_name = get_true_expr()->getVarName(); //varName that is used to search for register storing slt condition output
        dst<<"beqz "<<reg_ctxt.findVarReg(var_name)<<", "+elseLabel<<std::endl; //check if matches if condition
        reg_ctxt.emptyReg(var_name);
        //execute if body code
        func_scope->riscv(dst, reg_ctxt, freg_ctxt, Map);
        dst<<elseLabel+": "<<std::endl;
    }

};

class else_stmnt : public stmnt_node {
    private:
    decl_node* func_scope;
    public:
        else_stmnt(expr_node* _start_expr, expr_node* _true_expr, expr_node* _incr_expr, stmnt_node* _body, decl_node* _func_scope) :
        stmnt_node(_start_expr, _true_expr, _incr_expr, _body), func_scope(_func_scope)
        {}
    virtual void print(std::ostream& dst) override{
        //expected output: if (a<10) { a = a+1;}
        dst<<"else "; //might need to remove "{ }" as block statement includes it. or maybe block statement is just useless and we can use stmnt_list in parse instead.
        func_scope->print(dst);
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        //Assuming prior to else statemnet we had if statement, so we have jump to else label, so now mark else label:
        //dst<<"else: "<<std::endl; else label is at end of if statement, as we can't have else statement without if statement so will always have else label.
        //but we can have if statement without else

        //else statement body
        func_scope->riscv(dst, reg_ctxt, freg_ctxt, Map);
    }
};

class block_stmnt : public stmnt_node { //block_stmnt needs to have NodeListPntr input param so it can take "{" stmnt_list "}"
    public:
        block_stmnt(expr_node* _start_expr, expr_node* _true_expr, expr_node* _incr_expr, stmnt_node* _body) :
        stmnt_node(_start_expr, _true_expr, _incr_expr, _body)
        {}
    virtual void print(std::ostream& dst) override{
        //expected output: {body}
        dst<<"{";
        get_body()->print(dst);
        dst<<"}";
    }
    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        get_body()->riscv(dst, reg_ctxt, freg_ctxt, Map);
    }

};

class return_stmnt : public stmnt_node { //there's different types of returns: return, return expr
    public:
        return_stmnt(expr_node* _start_expr, expr_node* _true_expr, expr_node* _incr_expr, stmnt_node* _body) :
        stmnt_node(_start_expr, _true_expr, _incr_expr, _body)
        {}
    virtual void print(std::ostream& dst) override{
        //expected output: return
        dst<<"return;";
    }
    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        //not returning object/expression so just jump back to function call.
        //jump back to address stored in ra:
        dst<<"jr ra"<<std::endl;
    }

};

class return_stmnt_expr : public stmnt_node {
    private:
        bool flag;
    public:
        return_stmnt_expr(expr_node* _start_expr, expr_node* _true_expr, expr_node* _incr_expr, stmnt_node* _body, bool _flag) :
        stmnt_node(_start_expr, _true_expr, _incr_expr, _body), flag(_flag)
        {}

    virtual void print(std::ostream& dst) override{
        //expected output: return 1+3;
        dst<<"return"<<" ";
        get_true_expr()->print(dst);
        if(flag) {dst<<";"<<std::endl;}
        //dst<<std::endl;
    }
    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {
        get_true_expr()->riscv(dst, reg_ctxt, freg_ctxt, Map);

        int x = Map.calcStack(Map.getCurrFunc());
        //if expr is int_literal.
        //need to do something if expr is func call
        std::string resultReg = "";
        if (get_true_expr()->getVarName()=="empty") {resultReg = "a0";} //not sure if correct, we use this when doing function call as then we don't know variable name, but we know
        //return is always stored in a0 reg
        else {
            if(reg_ctxt.isVarInReg(get_true_expr()->getVarName())) {
                resultReg = reg_ctxt.findVarReg(get_true_expr()->getVarName());
                dst<<"mv a0, "<<resultReg<<std::endl;
                reg_ctxt.emptyReg(get_true_expr()->getVarName());
            }
            else if( freg_ctxt.isVarInReg(get_true_expr()->getVarName()) && freg_ctxt.VarType(get_true_expr()->getVarName())=="double" ) { //if return is double type
                resultReg = freg_ctxt.findVarReg(get_true_expr()->getVarName());
                dst<<"fmv.d fa0, "<<resultReg<<std::endl;
            }
            else if( freg_ctxt.isVarInReg(get_true_expr()->getVarName()) && freg_ctxt.VarType(get_true_expr()->getVarName())=="float" ) { //if return is double type
                resultReg = freg_ctxt.findVarReg(get_true_expr()->getVarName());
                dst<<"fmv.s fa0, "<<resultReg<<std::endl;
            }
        }

        //dst<<"mv a0, t0"<<std::endl; //first reg t0 is first one to be used to try to store return expression in this reg.
        //dst<<"jr ra"<<std::endl; //not sure if valid.
        dst<<"addi sp, sp, "<<x<<std::endl; // reset the stack, use the newly added current function method
        dst<<"jr ra "<<std::endl;
    }
};

class case_stmnt : public stmnt_node {
    private:
    NodeListPtr SubTreeListPtr = new NodeList();
    bool break_flag = false;
    public: //true_expr can be int literal
        case_stmnt(expr_node* _start_expr, expr_node* _true_expr, expr_node* _incr_expr, stmnt_node* _body, NodeListPtr _SubTreeListPtr, bool _break_flag) :
        stmnt_node(_start_expr, _true_expr, _incr_expr, _body), SubTreeListPtr(_SubTreeListPtr), break_flag(_break_flag)
        {}

    bool get_break_flag() {return break_flag;}

    virtual void print(std::ostream& dst) override{
        //expected output: if (a<10) { a = a+1;}

        if (get_true_expr()!=NULL) {
                dst<<"case ";
                get_true_expr()->print(dst);
                dst<<":"<<std::endl;
                if(SubTreeListPtr!=NULL) {
                    for (auto ptr: *SubTreeListPtr) {
                        ptr->print(dst);
                    }
                }
        }

        else { //so default
                dst<<"default ";
                dst<<":"<<std::endl;
                if(SubTreeListPtr!=NULL) {
                    for (auto ptr: *SubTreeListPtr) {
                        ptr->print(dst);
                    }
                }
            }
        }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map, std::string caseL) {

      //Instructions inside block:
      dst<<caseL+":"<<std::endl; //if default then no label

      if(SubTreeListPtr!=NULL) {
            for (auto ptr : *SubTreeListPtr) { //specifies instructions inside for loop definition.
                ptr->riscv(dst, reg_ctxt, freg_ctxt, Map);
            }
        }
    }

};

class switch_stmnt : public stmnt_node {
    private:
    CaseListPtr SubTreeListPtr = new CaseList();
    public: //true_expr can be the variable ident
        switch_stmnt(expr_node* _start_expr, expr_node* _true_expr, expr_node* _incr_expr, stmnt_node* _body, CaseListPtr _SubTreeListPtr) :
        stmnt_node(_start_expr, _true_expr, _incr_expr, _body), SubTreeListPtr(_SubTreeListPtr)
        {}
    virtual void print(std::ostream& dst) override{
        //expected output: if (a<10) { a = a+1;}
        dst<<"switch (";
        get_true_expr()->print(dst);
        dst<<")"<<std::endl;
        dst<<"{"<<std::endl;
        if(SubTreeListPtr!=NULL) {
            for (auto ptr: *SubTreeListPtr) {
                ptr->print(dst);
            }
        }
        dst<<"}"<<std::endl;
    }

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map) {

        std::string switch_end = makeName("switch_end");

        std::queue<std::string> caseLabelQ;

        bool default_flag = false;

        std::string default_label; //only one default label

        //So store case labels in queue, then pop from queue when executing case code:

        get_true_expr()->riscv(dst, reg_ctxt, freg_ctxt, Map);
        setVarName() = get_true_expr()->getVarName();

        //outputs all case conditions:
        if(SubTreeListPtr!=NULL) { //make SubTreeListPtr into CastStmntListPtr
            for (auto ptr : *SubTreeListPtr) {

                std::string caseLabel;
                if(ptr->get_true_expr()!=NULL) {
                    caseLabel = makeName("case");
                    //not every node* ptr has get_true_expr() func, so need to make it list of case_stmnt
                    ptr->get_true_expr()->riscv(dst, reg_ctxt, freg_ctxt, Map); //only do this is not default
                    //Comparison of identifier in switch case and all cases
                    dst<<"beq "<<reg_ctxt.findVarReg(getVarName())<<", "<<reg_ctxt.findVarReg(ptr->get_true_expr()->getVarName())<<", "+caseLabel<<std::endl;
                    caseLabelQ.push(caseLabel);
                }
                else { //so default
                    caseLabel = makeName("default");
                    default_label = caseLabel;
                    dst<<"j "+caseLabel<<std::endl; //jump to default
                    caseLabelQ.push(caseLabel);
                    default_flag = true;
                }
            }
        }

        if(default_flag==false) {
            //if we don't jump to any of these then jump to end:
            dst<<"j "+switch_end<<std::endl;
        }

        //outputs case code blocks if case is executed
        if(SubTreeListPtr!=NULL) { //make SubTreeListPtr into CastStmntListPtr
            for (auto ptr : *SubTreeListPtr) {
                auto caseL = caseLabelQ.front();
                caseLabelQ.pop();
                ptr->riscv(dst, reg_ctxt, freg_ctxt, Map, caseL);
                //if either case statement has break or is default then jump to end of switch
                if(ptr->get_break_flag()==true || ptr->get_true_expr()==NULL) { //checks if case has break or not.
                    dst<<"j "+switch_end<<std::endl;
                }
                else { //else jump to default
                    dst<<"j "+default_label<<std::endl;
                }
            }
        }
        dst<<switch_end+":"<<std::endl;
    }

};
class expr_stmnt : public stmnt_node {
    public:
        expr_stmnt(expr_node* _start_expr, expr_node* _true_expr, expr_node* _incr_expr, stmnt_node* _body) :
        stmnt_node(_start_expr, _true_expr, _incr_expr, _body)
        {}
    virtual void print(std::ostream& dst) override{
        //expected output: exor
        get_true_expr()->print(dst);
    }
    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST& Map) {
        get_true_expr()->riscv(dst, reg_ctxt, freg_ctxt, Map);
    }
};

#endif
