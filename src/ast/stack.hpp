#ifndef stack_hpp
#define stack_hpp
#include <string>
#include <map>
#include <algorithm>
#include <memory>

static int stackPtr = 0;

class stackAST{
    private:
        std::shared_ptr<std::map<std::string, std::vector<std::vector<variable_state>>>> stackMap;
        std::shared_ptr<std::string> curr_func;
        std::shared_ptr<std::map<std::string, std::vector<variable_state>>> structMap;
        std::shared_ptr<std::map<std::string, int>> stackSizeMap;
    public:
    //so we've intiailized an empty vector of integer vectors
    //so we're intiializing a map, making it a shared pntr, and assigning this pntr to stackMap member variable pntr
        stackAST() : stackMap(std::make_shared<std::map<std::string, std::vector<std::vector<variable_state>>>>()),
        curr_func(std::make_shared<std::string>("")),
        structMap(std::make_shared<std::map<std::string, std::vector<variable_state>>>()),
        stackSizeMap(std::make_shared<std::map<std::string, int>>()) //initialize empty map
        {}

    //This function only checks if variable already exists in current vector
        void incVector(std::vector<variable_state>& varList, std::string _name, varType _type, bool _isPntr = false, int array_size = 0){
            //by default isPntr = false
            //need to first check VarType and based on that increment stackPtr, as different types have different sizes
            if(_type==IntType) { //varType 0 is IntType
                if(!array_size){
                    stackPtr += 4;
                }
                else{
                    stackPtr += 4*array_size;
                }
            }
            else if (_type==DoubleType) { //varType is DoubleType
                if(!array_size){
                    stackPtr += 8;
                }
                else{
                    stackPtr += 8*array_size;
                }
            }
            else if (_type==FloatType) {
                if(!array_size){
                    stackPtr += 4;
                }
                else{
                    stackPtr += 4*array_size;
                }
            }
            else if (_type==CharType) {
                if(!array_size){
                    stackPtr += 1;
                }
                else{
                    stackPtr += 4*array_size;
                }
            }
            else {
                if(!array_size){
                    stackPtr += 4;
                }
                else{
                    stackPtr += 4*array_size;
                } //do 4 by default for now
            }

            variable_state var(_name, _type, "", _isPntr, stackPtr, true);

            if(std::none_of(varList.begin(), varList.end(), [&](auto& p){ return p.getName() == _name; })) {
                varList.push_back(var); //stores in a varList
            }
        }

    //function to add scope to binding so:
        void addScopeVarList(std::vector<std::vector<variable_state>>& scopesVarList, std::vector<variable_state>& varList) {
            scopesVarList.push_back(varList); //adds var list for a specific scope to list of all scopes in the function.
        }

    //need to have binding as: vector<vector<variable_state>>. //need to create func to add scope to binding
        void addBinding(const std::string& func_name, const std::vector<std::vector<variable_state>>& scopesVarList){
            if(stackMap->count(func_name) == 0){ //so func isn't in mapping
                (*stackMap)[func_name] = scopesVarList;
                (*stackSizeMap)[func_name] = stackPtr; //when doing binding also store func name and stack size in stackSizeMap:
                //as key-value pair doesnt exist, then statement above creates key-value pair.
            }
        }

        void resetStack(){
            stackPtr = 0;
        }

        void stackPrint(){
            for (auto& [funcName, scopesVarList]: *stackMap) { //[funcName, varList] allows us to refer to the key-value pair individually
                std::cout << "Function: " << funcName << std::endl;
                //Outputting all variables in scopesVarList:
                for (auto& varList : scopesVarList) {
                    for (auto& variable: varList) {
                        std::cout << "\tVariable: " << variable.getName() << ", Type: " << variable.getTypePrint() << ", Stack Position: " << variable.getStackMemAddr() <<", is Pointer: "<<variable.getIsPntr()<<std::endl;
                    }
                }
            }
        }
        //so add all variables for function binding into one vector and output that:
        std::vector<variable_state> returnVars(const std::string& func_name) const {
            if(stackMap->count(func_name) != 0){ //so function binding exists
                auto scopedVarList = (*stackMap)[func_name];
                auto allVars = std::vector<variable_state>();

                for (auto varList : scopedVarList) {
                    for (auto variable : varList) {
                        allVars.push_back(variable);
                    }
                }
                return allVars;

            }
            else {
                return std::vector<variable_state>();
            }
        }

        //so need to subtract StackMemAddr of final variable in final scoped list by first variable in first scoped list
        int calcStack(const std::string& func_name){ //outputs size of stack allocate for a function
            auto scopedVarList = (*stackMap)[func_name];
            if (scopedVarList.size()){ //non empty scopedVarList
                return (*stackSizeMap)[func_name]; //returns stackSize for specific func.
            }
            return 0;
        }

        void setCurrFunc(const std::string& func_name){
            (*curr_func) = func_name;
        }

        std::string getCurrFunc() const {
            return (*curr_func);
        }
        variable_state returnVar(const std::vector<variable_state>& varList, const std::string& _name){
            for(variable_state var : varList){
                if(var.getName()==_name){
                    return var;
                }
            }
            return variable_state();
        }

       //Helper function - Want to output iteartor to pair
       std::map<std::string, std::vector<std::vector<variable_state>>>::iterator findFuncName(const std::string& func_name) const {
            //check if func_name is in stackMap and if so return iterator to this pair (slightly different from returnVars func)
            if (stackMap->find(func_name) == stackMap->end()) {
                throw("function name called is not in stack map.");
            }
            return stackMap->find(func_name);
        }

        //LookUpVar given index of varList to look for
        //by default we look at global function scope so thats scope_index = 0
        std::vector<variable_state>::iterator lookUpVar(const std::string& func_name, const std::string& var_name, int scope_index=0) {

            auto it_Func = findFuncName(func_name); //iterator to func_name key
            auto varList = (it_Func->second)[scope_index]; //varList of that corresponding func at iterator it and at scope_index
            //Looking for var_name in varList

            auto compareName = [var_name] (variable_state a) { return var_name==a.getName(); }; //outputs true if a reg_state has same variable name as varName
            auto it_Var = find_if(varList.begin(), varList.end(), compareName); //iterator to variable
            if(it_Var==varList.end()) {
                throw("variable called for is not in variable table");
            }
            return it_Var; //not sure if it will still return vars.end(), if so then have to do lots of checks
        }

        varType lookUpVarType(const std::string& func_name, const std::string& var_name) {
            auto it = lookUpVar(func_name, var_name); //iterator to variable var_name
            return it->getType();
        }

        std::string lookUpVarTypePrint(const std::string& func_name, const std::string& var_name) {
            auto it = lookUpVar(func_name, var_name); //iterator to variable var_name
            return it->getTypePrint();
        }

        std::string lookUpVarReg(const std::string& func_name, const std::string& var_name) {
            auto it = lookUpVar(func_name, var_name);
            return it->getReg();
        }

        int lookUpVarStackAddr(const std::string& func_name, const std::string& var_name) {
            auto it = lookUpVar(func_name, var_name);
            return it->getStackMemAddr();
        }

        bool lookUpVarInStackMem(const std::string& func_name, const std::string& var_name) {
            auto it = lookUpVar(func_name, var_name);
            return it->getInStackMem();
        }

        //set to false for now - add once we start using pntr
        bool lookUpVarIsPntr(const std::string& func_name, const std::string& var_name) {
            auto it = lookUpVar(func_name, var_name);
            return it->getIsPntr();
        }

        ~stackAST() = default;
};

#endif