#ifndef ast_param_node_hpp
#define ast_param_node_hpp

#include <iostream> //uses public node from ast_node header file which is already in folder so no need for include
#include <string>
#include <vector>

class param_node : public node { //outptus list of parameters/arguments for func. Need to have args: args arglist thing in grammar to link params
    private:
        std::string name;
        //expr_node* val; //expr_node* as expr_int_literal and expr_node have same member variables, so we aren't discounting any member variables by doing expr_node*
        //cannot use expr_node* as it hasn't been defined as of yet in header file order
        int val;
        node* expr_param;
        type_node* type;
        param_node* next_param;
        bool pntr_flag;
    public:
        param_node(std::string _name, int _val, node* _expr_param, type_node* _type, param_node* _next_param, bool _pntr_flag = false) :
        name(_name), val(_val), expr_param(_expr_param), type(_type), next_param(_next_param), pntr_flag(_pntr_flag)
        {}

        int get_val(){
            return val;
        }

        virtual void print(std::ostream& dst) override{
            //Want to output int a, ... so:
            if(type!=NULL) {type->print(dst); dst<<" ";}

            if(val) {dst<<val;}
            else if (expr_param!=NULL) {expr_param->print(dst);}
            else {
                if(pntr_flag) {dst<<"*";}
                dst<<name;
            }

            if(next_param!=NULL) {
                dst<<", ";
                next_param->print(dst);}; //prints next argument in argument/parameter list} //so only if next_param isn't null it prints, if null then do nothing, so prevents seg fault.
            }

        virtual void stackBuild(stackAST &Map, std::vector<variable_state>& varList){
            //As params are all in 1 scope level:

            Map.incVector(varList, name, type->get_kind(), pntr_flag);

            if(next_param!=NULL){ next_param->stackBuild(Map, varList); }
        }

        //so this loads all integer literal input params into argument registers, not sure what to do for variable literals (might have to load from)
        //can have a flag that chooses whether to load params into arg registers a0, a1 (if function call) or if flag is false then store params in stack
        //currAReg is current argument reg to store in

    virtual void riscv(std::ostream& dst, register_context& reg_ctxt, fregister_context& freg_ctxt, stackAST &Map, bool store_flag, int currAReg, int currAFReg, std::vector<std::string>& noResetRegs) { //store params in certain registers

        auto func_name = Map.getCurrFunc();

        if(store_flag==true) { //if function definition then want to store arguments into memory.

            //Checking type:
            if(type->getTypePrint()=="int") {
                //reg_ctxt.newVar(name, IntType, "a0", "a7"); //assigns input param variable name to a free register
                //Store params immediately in stack and empty used reg:
                auto func_name = Map.getCurrFunc(); //string storing current func_name
                auto arg_reg = "a"+std::to_string(currAReg);
                dst<<"sw "<<arg_reg<<", "<<Map.lookUpVarStackAddr(func_name, name)-4<<"(sp)"<<std::endl;
                //dst<<"sw "<<reg_ctxt.findVarReg(name)<<", "<<Map.lookUpVarStackAddr(func_name, name)-4<<"(sp)"<<std::endl;
                //So traverse through all argument registers and store arguments in memory and empty then, but empty such that we don't use the next available register in the next argument storing.
                //This is only exception for function parameters
                reg_ctxt.emptyRegName(arg_reg); //so always empty reg so it's available
                currAReg++;
            }
            else if(type->getTypePrint()=="double") {
                auto func_name = Map.getCurrFunc(); //string storing current func_name
                auto arg_reg = "fa"+std::to_string(currAFReg);
                dst<<"fsd "<<arg_reg<<", "<<Map.lookUpVarStackAddr(func_name, name)-8<<"(sp)"<<std::endl;
                //dst<<"sw "<<reg_ctxt.findVarReg(name)<<", "<<Map.lookUpVarStackAddr(func_name, name)-4<<"(sp)"<<std::endl;
                //So traverse through all argument registers and store arguments in memory and empty then, but empty such that we don't use the next available register in the next argument storing.
                //This is only exception for function parameters
                freg_ctxt.emptyRegName(arg_reg); //so always empty reg so it's available
                currAFReg++;
            } //do similar for loading, so below stuff:
            else if(type->getTypePrint()=="float") {
                auto func_name = Map.getCurrFunc(); //string storing current func_name
                auto arg_reg = "fa"+std::to_string(currAFReg);
                dst<<"fsw "<<arg_reg<<", "<<Map.lookUpVarStackAddr(func_name, name)-4<<"(sp)"<<std::endl;
                //dst<<"sw "<<reg_ctxt.findVarReg(name)<<", "<<Map.lookUpVarStackAddr(func_name, name)-4<<"(sp)"<<std::endl;
                //So traverse through all argument registers and store arguments in memory and empty then, but empty such that we don't use the next available register in the next argument storing.
                //This is only exception for function parameters
                freg_ctxt.emptyRegName(arg_reg); //so always empty reg so it's available
                currAFReg++;
            } //do similar for loading, so below stuff:
        }
        else { //else we're loading into regs
            //we load onto a0 reg so: if input param is a variale:
            if(expr_param!=NULL) {
                //so an input param is an expr like: (n-1) then do expr operation and lw into arg:
                expr_param->riscv(dst, reg_ctxt, freg_ctxt, Map);
                dst<<"mv "<<"a"+std::to_string(currAReg)<<", "<<reg_ctxt.findVarReg(expr_param->getVarName())<<std::endl;
                //do push_back into noResetRegs vector(reg_ctxt.findVarReg(expr_param->getVarName())) and reset regs after next_param and pass on this noResetRegs vector into the function call
                noResetRegs.push_back(reg_ctxt.findVarReg(expr_param->getVarName()));
            }
            else if(name!="empty") { //so input param must be variables
                dst<<"lw "<<"a"+std::to_string(currAReg)<<", "<<Map.lookUpVarStackAddr(func_name, name)-4<<"(sp)"<<std::endl;
            }
            else { //input param is number
                dst<<"li "<<"a"+std::to_string(currAReg)<<", "<<val<<std::endl;
            }
            currAReg++;

            //after executing function call WHERE WE'RE LOADING VALUES INTO ARGUMENT REGISTERS: a0, a1.
            //if recursive or not we want to reset all registers except of the argument regs:
            //emptyRegName() //for now test by just freeing all regs except a0.

            //Make this after if(next_param!=NULL){...}

            //Add ra reg:
            //noResetRegs.push_back("ra");
            //reg_ctxt.resetAllRegs(noResetRegs); //this reset regs before jumping to function call

        }
            //not sure if too free reg or not

        if(next_param!=NULL) {
            //dst<<std::endl;
            next_param->riscv(dst, reg_ctxt, freg_ctxt, Map, store_flag, currAReg, currAFReg, noResetRegs);
        }

        //reg_ctxt.resetAllRegs(noResetRegs);

    }

    virtual int count_params(int count){ // needed for arrays, to count the number of array elements
        if(next_param != NULL){
            return next_param->count_params(count+1);
        }
        return count;
    }

        virtual ~param_node() {
            delete type;
            delete next_param;
            delete expr_param;
        }
};

#endif