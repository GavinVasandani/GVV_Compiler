%code requires{
    //include correct dependencies - AST node file
  #include "ast.hpp"

  #include <iostream>
  #include <string>
  #include <cassert>

  extern node* g_root; // A way of getting the AST out, we output head node which is decl_node type.
  extern FILE *yyin;

  //! This is to fix problems when generating C++
  // We are declaring the functions provided by Flex, so
  // that Bison generated code can call them.
  int yylex(void);
  void yyerror(const char *);
}

%union{
    decl_node* decl;
    stmnt_node* stmnt;
    expr_node* expr;
    type_node* types;
    param_node* param;
    int number;
    std::string* word;
    NodeListPtr ListPntr;
    main_node* main;
    nodeptr node;
    case_stmnt* caseType;
    CaseListPtr CListPntr;
}

%token <word> IDENTIFIER STRING_ASSIGN FUNCTION_NAME CHAR SHORT INT LONG FLOAT DOUBLE VOID STRING UNSIGNED CHAR_ASSIGN DOUBLE_LITERAL FLOAT_LITERAL STRUCT //make types string* types
%token <number> NUMBER
%token IF ELSE FOR RETURN WHILE IS_EQUAL LT_EQUAL GT_EQUAL LOGICAL_AND LOGICAL_OR EMPTY_LINE SIZEOF SWITCH CASE INCREMENT DECREMENT BREAK DEFAULT RIGHT_BITSHIFT INCREMENT_CUSTOM
%token ';' '{' '}' '(' ')' ',' ':' '+' '-' '/' '*' '[' ']' '|' '&' '^' '=' '?' '!' '>' '<'

%type <decl> root decl func_statement var_statement array_statement scoped_statement pointer_statement struct_statement
%type <main> main_list
%type <ListPntr> decl_list block_list block_list_case
%type <CListPntr> case_list
%type <stmnt> loop_statement conditional_statement return_statement expr_statement switch_statement
%type <expr> expr expr_form arithmetic_expr assign_expr bitwise_expr condition_expr literal_expr identifier_expr array_expr sizeof_expr
%type <types> type_specifier
%type <param> arguments
%type <caseType> case
%type <node> block block_case

%left '|' '^'
%left '&'
%left '<' '>' LT_EQUAL GT_EQUAL IS_EQUAL
%left RIGHT_BITSHIFT
%left '+' '-'
%left '*' '/'
%left UMINUS
%left LOGICAL_AND
%left LOGICAL_OR
%left PAREN
%left '(' ')'
%left '[' ']'
%start root

%%

root : main_list {g_root = $1;} //root of AST is node* type can be assigned to decl_node* type

main_list
    : decl_list {$$ = new main_node($1);}
    ;

decl_list
    : decl {$$ = create_list($1);}
    | decl_list decl {$$ = append_list($1, $2);}
    | decl_list EMPTY_LINE decl {$$ = append_list($1, $3);}
    | EMPTY_LINE {$$ = NULL;}
    ;

decl
    : func_statement {$$ = $1;}
    | array_statement {$$ = $1;}
    | scoped_statement {$$ = $1;}
    | struct_statement {$$ = $1;}
    ;

struct_statement
    : STRUCT IDENTIFIER'{' arguments '}' ';' {$$ = new struct_decl(*$2, NULL, NULL, $4, "");}
    | STRUCT IDENTIFIER IDENTIFIER ';' {$$ = new struct_decl(*$2, NULL, NULL, NULL, *$3);}
    ;

func_statement
    : type_specifier IDENTIFIER '(' ')' ';' {$$ = new func_decl(*$2, $1, NULL, NULL);}  //func declaration with no input args - WORKS
    | type_specifier IDENTIFIER '(' arguments ')' ';' {$$ = new func_decl(*$2, $1, NULL, $4);} //function prototype. NULL member variables' print funcs aren't being called so no seg fault. - WORKS
    | type_specifier IDENTIFIER '(' arguments ')' scoped_statement {$$ = new func_def(*$2, $1, NULL, $4, $6);} //statemnet_list must be stmnt node type. Need to somehow pass args list to type_specifier or modify create_decl_node - WORKS
    | type_specifier IDENTIFIER '(' ')' scoped_statement {$$ = new func_def(*$2, $1, NULL, NULL, $5);} //NEW: this defines a function with no arguments passed. $5 is passed for the block statements part, there are no arguments so they are set to null. - WORKS
    ;

scoped_statement
    : '{' block_list '}' {$$ = new scope_def("0", NULL, NULL, NULL, $2);}
    ;

array_statement
    : type_specifier IDENTIFIER '[' expr ']' '=' '{' arguments '}' ';' { $$ = new array_def(*$2, $1, $4, $8); }
    | type_specifier IDENTIFIER '['']' '=' '{' arguments '}' ';' { $$ = new array_def(*$2, $1, NULL, $7); }
    | type_specifier IDENTIFIER '[' expr ']' ';' {$$ = new array_decl(*$2, $1, $4, NULL);}
    ;

block_list
    : block {$$ = create_list($1);}
    | block_list block {$$ = append_list($1, $2);}
    | block_list EMPTY_LINE block {$$ = append_list($1, $3);}
    | EMPTY_LINE {$$ = NULL;}
    ;
        /*EMPTY_LINE is new token added, works well with test cases but test more to see it doesn't interfere with other cases*/

block_list_case
    : block_case {$$ = create_list($1);}
    | block_list_case block_case {$$ = append_list($1, $2);}
    | block_list_case EMPTY_LINE block_case {$$ = append_list($1, $3);}
    | EMPTY_LINE {$$ = NULL;}
    ;

block
    : var_statement {$$ = $1;}
    | loop_statement {$$ = $1;}
    | return_statement {$$ = $1;}
    | conditional_statement {$$ = $1;}
    | expr_statement {$$ = $1;}
    | switch_statement {$$ = $1;}
    | decl {$$ = $1;}
    | pointer_statement {$$ = $1;}
    ;

block_case
    : var_statement {$$ = $1;}
    | loop_statement {$$ = $1;}
    | return_statement {$$ = $1;}
    | conditional_statement {$$ = $1;}
    | expr_statement {$$ = $1;}
    | decl {$$ = $1;}
    | pointer_statement {$$ = $1;}
    ;

case
    : CASE literal_expr ':' block_list_case {$$ = new case_stmnt(NULL, $2, NULL, NULL, $4, false);}
    | CASE literal_expr ':' block_list_case BREAK ';' {$$ = new case_stmnt(NULL, $2, NULL, NULL, $4, true);}
    | DEFAULT ':' block_list_case {$$ = new case_stmnt(NULL, NULL, NULL, NULL, $3, false);}
    ;

case_list
    : case {$$ = create_case_list($1);}
    | case_list case {$$ = append_case_list($1, $2);}
    | EMPTY_LINE {$$ = NULL;}
    ;

var_statement
    : type_specifier IDENTIFIER ';' {$$ = new var_decl(*$2, $1, NULL, NULL);} //i.e. int a; //pntr to next decl is NULL as we don't have next decl on same line as we have all parameters in param list on the same line - WORKS
    | type_specifier IDENTIFIER '=' expr ';' {$$ = new var_def(*$2, $1, $4, NULL);} //i.e. int a = 5; expr breaks down to leaf with constant int - WORKS
    ;

pointer_statement
    : type_specifier '*' IDENTIFIER ';' {$$ = new pntr_decl(*$3, $1, NULL, NULL);}
    | type_specifier '*' IDENTIFIER '=' expr ';' {$$ = new pntr_def(*$3, $1, $5, NULL);}
    ;

loop_statement
    : FOR '(' expr ';' expr ';' expr')' scoped_statement {$$ = new for_stmnt($3, $5, $7, NULL, $9);} //create_stmnt_node(for_stmnt, NULL, $3, $5, $7, $9, NULL, NULL);} - WORKS
    | WHILE '(' expr ')' scoped_statement {$$ = new while_stmnt(NULL, $3, NULL, NULL, $5);}
    ;

conditional_statement
    : IF '(' expr ')' scoped_statement {$$ = new if_stmnt(NULL, $3, NULL, NULL, $5);} //create_stmnt_node(if_stmnt, NULL, NULL, $3, NULL, $5, NULL, NULL);} //i.e if (i<10) {a=3;} //maybe replace statement_list with block_statement
    | ELSE scoped_statement {$$ = new else_stmnt(NULL, NULL, NULL, NULL, $2);}
    ;

return_statement
    : RETURN ';' {$$ = new return_stmnt(NULL, NULL, NULL, NULL);} //just returns from func/scope - WORKS
    | RETURN expr ';' {$$ = new return_stmnt_expr(NULL, $2, NULL, NULL, true);} //Referencing an already declared variable can be done by expr so can just be RETURN expr. - WORKS
    | RETURN expr {$$ = new return_stmnt_expr(NULL, $2, NULL, NULL, false);} //Referencing an already declared variable can be done by expr so can just be RETURN expr. - WORKS
    ;

expr_statement
    : expr {$$ = new expr_stmnt(NULL, $1, NULL, NULL);} //WORKS
    | expr ';' {$$ = new expr_stmnt(NULL, $1, NULL, NULL);} //WORK
    ;

switch_statement
    : SWITCH '(' identifier_expr ')' '{' case_list '}' {$$ = new switch_stmnt(NULL, $3, NULL, NULL, $6);}
    ;

expr
    : expr_form {$$ = $1;}
    | '(' expr_form ')' %prec PAREN{$$ = $2;}
    ;

expr_form
    : arithmetic_expr {$$ = $1;}
    | assign_expr {$$ = $1;}
    | bitwise_expr {$$ = $1;}
    | condition_expr {$$ = $1;}
    | literal_expr {$$ = $1;}
    | identifier_expr {$$ = $1;}
    | array_expr {$$ = $1;}
    | sizeof_expr { }
    ;

sizeof_expr
    : SIZEOF '(' expr ')' { $$ = new expr_sizeof($3, NULL, "0", 0, "0", NULL); }
    | SIZEOF '(' type_specifier ')' { $$ = new expr_sizeof(NULL, NULL, "0", 0, "0", $3); }
    ;

array_expr
    : IDENTIFIER '[' expr ']' '=' expr {$$ = new expr_array($3, $6, *$1, 0, "0");}
    | IDENTIFIER '[' expr ']' {$$ = new expr_array($3, NULL, *$1, 0, "0");}
    ;

arithmetic_expr
    : '-' expr %prec UMINUS{$$ = new expr_neg(NULL, $2, "0", 0, "0");}
    | expr '+' expr {$$ = new expr_add($1, $3, "0", 0, "0");} //WORKS, x & y + 3 is possible so make non-terminals expr.
    | expr '-' expr {$$ = new expr_sub($1, $3, "0", 0, "0");} //WORKS
    | expr '*' expr {$$ = new expr_mul($1, $3, "0", 0, "0");} //WORKS
    | expr '/' expr {$$ = new expr_div($1, $3, "0", 0, "0");} //WORKS
    | expr INCREMENT {$$ = new expr_incr($1, NULL, "0", 0, "0");}
    | expr INCREMENT_CUSTOM expr {$$ = new expr_incr_cust($1, $3, "0", 0, "0");}
    | expr DECREMENT {$$ = new expr_decr($1, NULL, "0", 0, "0");}
    | expr RIGHT_BITSHIFT literal_expr {$$ = new expr_right_bitshift($1, $3, "0", 0, "0");}
    ;

assign_expr
    : expr '=' expr {$$ = new expr_assign($1, $3, "0", 0, "0");} //WORKS
    ;

bitwise_expr
    : expr '&' expr {$$ = new expr_bitwise_AND($1, $3, "0", 0, "0");} //WORKS
    | expr '|' expr {$$ = new expr_bitwise_OR($1, $3, "0", 0, "0");} //WORKS
    | expr '^' expr {$$ = new expr_bitwise_XOR($1, $3, "0", 0, "0");} //WORKS
    ;

condition_expr
    : expr IS_EQUAL expr {$$ = new expr_is_equal($1, $3, "0", 0, "0");}
    | expr '<' expr {$$ = new expr_less_than($1, $3, "0", 0, "0");} //WORKS
    | expr LT_EQUAL expr {$$ = new expr_lt_equal($1, $3, "0", 0, "0");} //WORKS
    | expr '>' expr {$$ = new expr_greater_than($1, $3, "0", 0, "0");} //WORKS
    | expr GT_EQUAL expr {$$ = new expr_gt_equal($1, $3, "0", 0, "0");} //WORKS
    | expr LOGICAL_AND expr {$$ = new expr_logical_and($1, $3, "0", 0, "0");} //WORKS
    | expr LOGICAL_OR expr {$$ = new expr_logical_or($1, $3, "0", 0, "0");} //WORKS
    ;

literal_expr
    : NUMBER {$$ = new expr_int_literal(NULL, NULL, "0", $1, "0");}  //int constant. leaf node.
    | CHAR_ASSIGN {$$ = new expr_char_literal(NULL, NULL, "0", 0, *$1); }
    | STRING_ASSIGN {$$ = new expr_string_literal(NULL, NULL, "0", 0, *$1);} //assuming string_assign is string literal.
    | DOUBLE_LITERAL {$$ = new expr_double_literal(NULL, NULL, "0", 0, "0", *$1);}
    | FLOAT_LITERAL {$$ = new expr_float_literal(NULL, NULL, "0", 0, "0", *$1); }
    ;

identifier_expr
    : IDENTIFIER '(' arguments ')' {$$ = new expr_func_call(NULL, NULL, *$1, 0, "0", $3);} //function call grammar
    | IDENTIFIER {$$ = new expr_ident(NULL, NULL, *$1, 0, "0");}  //dereferences variable identifier. This variable must have already been previously declared. As leaf node, we use different create func
    | IDENTIFIER '(' ')' {$$ = new expr_func_call(NULL, NULL, *$1, 0, "0", NULL);} //function call grammar
    | '&' IDENTIFIER {$$ = new expr_addr_ident(NULL, NULL, *$2, 0, "0");}
    | '*' IDENTIFIER {$$ = new expr_deref_ident(NULL, NULL, *$2, 0, "0");}
    ;

type_specifier
    : INT               {varType _kind = IntType; $$ = new type_node(_kind);} //WORKS
    | DOUBLE            {varType _kind = DoubleType; $$ = new type_node(_kind);}
    | LONG              {varType _kind = LongType; $$ = new type_node(_kind);}
    | SHORT             {varType _kind = ShortType; $$ = new type_node(_kind);}
    | FLOAT             {varType _kind = FloatType; $$ = new type_node(_kind);}
    | CHAR              {varType _kind = CharType; $$ = new type_node(_kind);}
    | STRING            {varType _kind = StringType; $$ = new type_node(_kind);}
    | VOID              {varType _kind = VoidType; $$ = new type_node(_kind);}
    | UNSIGNED          {varType _kind = UnsignedType; $$ = new type_node(_kind);}
    ;

arguments
    : arithmetic_expr ',' arguments {$$ = new param_node("empty", 0, $1, NULL, $3);}
    | arithmetic_expr {$$ = new param_node("empty", 0, $1, NULL, NULL);}
    | type_specifier IDENTIFIER ',' arguments {$$ = new param_node(*$2, 0, NULL, $1, $4);}
    | type_specifier IDENTIFIER {$$ = new param_node(*$2, 0, NULL, $1, NULL);}
    | type_specifier '*' IDENTIFIER ',' arguments {$$ = new param_node(*$3, 0, NULL, $1, $5, true);}
    | type_specifier '*' IDENTIFIER {$$ = new param_node(*$3, 0, NULL, $1, NULL, true);}
    | IDENTIFIER ',' arguments {$$ = new param_node(*$1, 0, NULL, NULL, $3);}
    | IDENTIFIER {$$ = new param_node(*$1, 0, NULL, NULL, NULL);}
    | NUMBER ',' arguments {$$ = new param_node("empty", $1, NULL, NULL, $3);}
    | NUMBER {$$ = new param_node("empty", $1, NULL, NULL, NULL);}
    | type_specifier IDENTIFIER ';' arguments {$$ = new param_node(*$2, 0, NULL, $1, $4);}
    | type_specifier IDENTIFIER ';' {$$ = new param_node(*$2, 0, NULL, $1, NULL);}
    ;

%%

node* g_root; // Definition of variable (to match declaration earlier)

node* parseAST(std::string filename)
{ //filename is the input c program
  yyin = fopen(filename.c_str(), "r"); //takes in input file from lexer (?)
  if (yyin == NULL) {
    std::cerr << "Couldn't open input file: " << filename << std::endl;
    exit(1);
  }
  g_root = NULL; //yyin isn't null so assign g_root as NULL. Execute parser code. Then evaluate g_root if (1) then parse successful
  //Which node is assigned head node?
  yyparse();
  return g_root;
}
