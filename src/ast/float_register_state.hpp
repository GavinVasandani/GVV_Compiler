#ifndef float_register_state_hpp
#define float_register_state_hpp

//Program that tells us the current state of registers, so: what registers are used, are registers available, assigning value to an available register
#include <ctime>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>

struct freg_state {
    //can have same names as reg_state, as its in its own scope it doesn't interfere
    std::string regName;
    bool regUsed;
    variable_state var; //information about current variable stored in register
    freg_state(std::string _regName, bool _regUsed, variable_state _var) : regName(_regName), regUsed(_regUsed), var(_var) {}
    //getters:
    std::string getregName() {return regName;}
    bool getregUsed() {return regUsed;}
    variable_state getVar() {return var;}
    //setters:
    std::string& setregName() {return regName;}
    bool& setregUsed() {return regUsed;}
    variable_state& setVar() {return var;}
};

class fregister_context {
    //These method functions can only be called by fregister_context object so can be same name as methods of register_context.

    private:
        //All possible register names for RISCV ABI
    std::vector<freg_state> regs = {freg_state("ft0", false, variable_state()), freg_state("ft1", false, variable_state()), freg_state("ft2", false, variable_state()), freg_state("ft3", false, variable_state()), freg_state("ft4", false, variable_state()), freg_state("ft5", false, variable_state()), 
        freg_state("ft6", false, variable_state()), freg_state("ft7", false, variable_state()), freg_state("fs0", false, variable_state()), freg_state("fs1", false, variable_state()), freg_state("fa0", false, variable_state()), freg_state("fa1", false, variable_state()), freg_state("fa2", false, variable_state()), 
        freg_state("fa3", false, variable_state()), freg_state("fa4", false, variable_state()), freg_state("fa5", false, variable_state()), freg_state("fa6", false, variable_state()), freg_state("fa7", false, variable_state()), freg_state("fs2", false, variable_state()), freg_state("fs3", false, variable_state()), 
        freg_state("fs4", false, variable_state()), freg_state("fs5", false, variable_state()), freg_state("fs6", false, variable_state()), freg_state("fs7", false, variable_state()), freg_state("fs8", false, variable_state()), freg_state("fs9", false, variable_state()), freg_state("fs10", false, variable_state()), 
        freg_state("fs11", false, variable_state()), freg_state("ft8", false, variable_state()), freg_state("ft9", false, variable_state()), freg_state("ft10", false, variable_state()), freg_state("ft11", false, variable_state())};

    //temp_regs (t0-t6) store values that can be rewritten by a function call, so these values aren't preserved over function calls. So store value in mem then do function call where t0-t2 regs will most likely be reassigned

    //general_regs (s0-s11) store values that must be saved and restored by function call, so whatever value is in these registers before a function call is the same value in the registers after function call.
    //So register to return a function call result in is most likely temp_regs as its value doesn't have to be restored.

    //arguments/params (a0-a7) to function call are passed in these regs.
    public:

    //find funcs:

    //outputs register name given its index
    std::string findRegName(int index) {
        return regs[index].getregName();
    }

    //Finds register index given register name
    int findRegIndex(std::string name) {
        for (int i=0; i<regs.size(); i++) {
            if(regs[i].getregName()==name) {
                return i;
            }
        }
        throw("fregister cannot be found!");
        return -1;
    }

    bool regEmpty(std::string name) {
        int ind = findRegIndex(name);
        return !regs[ind].getregUsed(); //so if used then outputs false for empty check
    }

    //findfreereg func for each reg type?
    //FindFreeRegister: output reg index not reg name
    int findFreeReg(std::string startRegName, std::string endRegName) {
        //only looks for empty registers within a range of registers given by startRegName - endRegName
        int startRegInd = findRegIndex(startRegName);
        int endRegInd = findRegIndex(endRegName);

        for (int i=startRegInd; i<=endRegInd; i++) {
            if (regEmpty(regs[i].getregName())) {
                //so find empty register in range of possible registers between t0 - a7, if found then change reg to regUsed = true;
                regs[i].setregUsed() = true;
                //output reg name: i.e. t1
                //return regs[i].getregName();
                return i;
            }
        }

        //if no empty register then reassign value in register:
        //for now store value in random register but optimize for storing in least often used register (maybe track number of times register has been used?)
        //srand(time(0));
        //auto random_ind = (rand() % (endRegInd - startRegInd + 1)) + startRegInd;
        //return regs[random_ind].getregName();
        //return random_ind;
        //add property for: CanBeUsed() to determine if register is the most optimal one to be reassigned.
    }

    //makes a reg empty given a variable name, so searches for the variable in the registers and finds the register that stores the variable and empties it
    void emptyReg(std::string varName) {
        auto registerName = findVarReg(varName); //registerName is name of register that stores variable
        int ind = findRegIndex(registerName);
        regs[ind].setregUsed() = false;
    }

    //empties reg given regname
    void emptyRegName(std::string registerName) {
        int ind = findRegIndex(registerName);
        regs[ind].setregUsed() = false;
    }

    void newVar(std::string varName, varType kind, std::string startRegName, std::string endRegName) {
        //need to create a vector that tracks all variable declarations (aka all variables alive) in current function scope.
        //want to store var in register for now (actually start by storing in stack):
        auto storeRegInd = findFreeReg(startRegName, endRegName);

        //Reassign properties of var:
        //First create variable_state obj and assign its name, type and register it will be stored at:
        auto new_var = variable_state(varName, kind, regs[storeRegInd].getregName());
        //Add this to reg property:
        //regs vector is a mirror of what registers are taken in RISCV so we can see where things are and allocate instructions accordingly
        //set reg as now used:
        regs[storeRegInd].setregUsed() = true;
        regs[storeRegInd].setVar() = new_var;

        //Output instruction to write value to register:
        //dst<<"lw "<<regs[storeRegInd].getregName()<<", "<< need to load variable onto stack then pull into register. so still have to finish.
    }

    //Function that searches which register contains the variable name and returns the register name:
    std::string findVarReg(std::string varName) {
        //if (varName == "a0") {return "a0";} //this is bad, should not be here but just doing for exceptional case. NEED TO remove.

        auto compareName = [varName] (freg_state a) { return varName==a.getVar().getName() && a.getregUsed(); }; //outputs true if a reg_state has same variable name as varName
        auto it = find_if(regs.begin(), regs.end(), compareName);

        if (it==regs.end()) {
            throw("fregister cannot be found!");
            return "error";
        }

        int ind = distance(regs.begin(), it);
        return regs[ind].getregName();
    }

    variable_state getVarType(std::string varName) {
        //if (varName == "a0") {return "a0";} //this is bad, should not be here but just doing for exceptional case. NEED TO remove.

        auto compareName = [varName] (freg_state a) { return varName==a.getVar().getName() && a.getregUsed(); }; //outputs true if a reg_state has same variable name as varName
        auto it = find_if(regs.begin(), regs.end(), compareName);

        if (it==regs.end()) {
            throw("fregister cannot be found!");
            return variable_state();
        }

        int ind = distance(regs.begin(), it);
        return regs[ind].getVar();
    }

    bool isVarInReg(std::string varName) {
        auto compareName = [varName] (freg_state a) { return varName==a.getVar().getName() && a.getregUsed(); }; //outputs true if a reg_state has same variable name as varName
        auto it = find_if(regs.begin(), regs.end(), compareName);

        if (it==regs.end()) { //if iterator is end then VarIsNotInReg so false
            return false;
        }

        return true; //so if iterator is not end then VarIsInReg so true
    }

    std::string VarType(std::string varName) {
        auto getV = getVarType(varName); //outputs variable_state, to get variable_state's type do:
        return getV.getTypePrint();
    }

};

#endif