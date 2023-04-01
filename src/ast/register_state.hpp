#ifndef register_state_hpp
#define register_state_hpp

//Program that tells us the current state of registers, so: what registers are used, are registers available, assigning value to an available register
#include <ctime>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>

struct reg_state {
    std::string regName;
    bool regUsed;
    variable_state var; //information about current variable stored in register
    reg_state(std::string _regName, bool _regUsed, variable_state _var) : regName(_regName), regUsed(_regUsed), var(_var) {}
    //getters:
    std::string getregName() {return regName;}
    bool getregUsed() {return regUsed;}
    variable_state getVar() {return var;}
    //setters:
    std::string& setregName() {return regName;}
    bool& setregUsed() {return regUsed;}
    variable_state& setVar() {return var;}
};

class register_context {

    private:
        //All possible register names for RISCV ABI
    std::vector<reg_state> regs = {reg_state("zero", true, variable_state()), reg_state("ra", true, variable_state()), reg_state("sp", false, variable_state()), reg_state("gp", false, variable_state()), reg_state("tp", false, variable_state()), reg_state("t0", false, variable_state()),
        reg_state("t1", false, variable_state()), reg_state("t2", false, variable_state()), reg_state("t3", false, variable_state()), reg_state("t4", false, variable_state()), reg_state("t5", false, variable_state()), reg_state("t6", false, variable_state()), reg_state("s0", false, variable_state()),
        reg_state("s1", false, variable_state()), reg_state("s2", false, variable_state()), reg_state("s3", false, variable_state()), reg_state("s4", false, variable_state()), reg_state("s5", false, variable_state()), reg_state("s6", false, variable_state()), reg_state("s7", false, variable_state()),
        reg_state("s8", false, variable_state()), reg_state("s9", false, variable_state()), reg_state("s10", false, variable_state()), reg_state("s11", false, variable_state()), reg_state("a0", false, variable_state()), reg_state("a1", false, variable_state()), reg_state("a2", false, variable_state()),
        reg_state("a3", false, variable_state()), reg_state("a4", false, variable_state()), reg_state("a5", false, variable_state()), reg_state("a6", false, variable_state()), reg_state("a7", false, variable_state())};

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
        throw("register cannot be found!");
        return -1;
    }

    bool regEmpty(std::string name) { //checks whether a register is empty given register name: a0, a1 etc.
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

    //empties reg given name of register i.e. a0, a1 etc.
    void emptyRegName(std::string registerName) {
        int ind = findRegIndex(registerName);
        regs[ind].setregUsed() = false;
    }

    void newVar(std::string varName, varType kind, std::string startRegName, std::string endRegName, bool _isPntr = false) {
        //need to create a vector that tracks all variable declarations (aka all variables alive) in current function scope.
        //want to store var in register for now (actually start by storing in stack):
        auto storeRegInd = findFreeReg(startRegName, endRegName);

        //Reassign properties of var:
        //First create variable_state obj and assign its name, type and register it will be stored at:
        auto new_var = variable_state(varName, kind, regs[storeRegInd].getregName(), _isPntr);
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

        auto compareName = [varName] (reg_state a) { return varName==a.getVar().getName() && a.getregUsed(); }; //outputs true if a reg_state has same variable name as varName
        auto it = find_if(regs.begin(), regs.end(), compareName);

        if (it==regs.end()) {
            throw("register cannot be found!");
            return "error";
        }

        int ind = distance(regs.begin(), it);
        return regs[ind].getregName();
    }

    variable_state getVarType(std::string varName) { //general function that outputs the variable_state given the variable name
        //if (varName == "a0") {return "a0";} //this is bad, should not be here but just doing for exceptional case. NEED TO remove.

        auto compareName = [varName] (reg_state a) { return varName==a.getVar().getName() && a.getregUsed(); }; //outputs true if a reg_state has same variable name as varName
        auto it = find_if(regs.begin(), regs.end(), compareName);

        if (it==regs.end()) {
            throw("register cannot be found!");
            return variable_state();
        }

        int ind = distance(regs.begin(), it);
        return regs[ind].getVar();
    }

    bool isVarInReg(std::string varName) {
        auto compareName = [varName] (reg_state a) { return varName==a.getVar().getName() && a.getregUsed(); }; //outputs true if a reg_state has same variable name as varName
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

    bool isVarPntr(std::string varName) { //outputs whether variable isPntr or not given variable name
        auto getV = getVarType(varName); 
        return getV.getIsPntr();
    }

    void resetAllRegs(std::vector<std::string> noResetRegs) { //use input param: std::vector<std::string> noResetRegs
        //Function that resets all registers except of register names in vector noResetRegs:

        //For now just reset all regs except a0 reg.
        //So loop through regs vector, if name is in noResetRegs then don't emptyReg

        auto regsBegin = regs.begin();
        auto regsEnd = regs.end();
        int i = 0; //i is index of noResetRegs vector

        for(auto it=regsBegin; it!=regsEnd; it++) {
            if(it->getregName() == noResetRegs[i]) {
                i++; //iterate to next register in noResetRegs
                continue; //do next iteration
            }
            else { //so want to empty
                emptyRegName(it->getregName());
            }
        }
    }

    void resetAllRegsExceptOne(std::string noResetReg) { //use input param: std::vector<std::string> noResetRegs
        //Function that resets all registers except of register names in vector noResetRegs:

        //For now just reset all regs except a0 reg.
        //So loop through regs vector, if name is in noResetRegs then don't emptyReg

        auto regsBegin = regs.begin();
        auto regsEnd = regs.end();

        for(auto it=regsBegin; it!=regsEnd; it++) {
            if(it->getregName() == noResetReg) {
                //iterate to next register in noResetRegs
                continue; //do next iteration
            }
            else { //so want to empty
                emptyRegName(it->getregName());
            }
        }
    }

};

#endif
