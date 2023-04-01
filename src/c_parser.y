%code requires{
    //include correct dependencies - AST node file

  #include <cassert>

  // extern const AST_node* g_root; // A way of getting the AST out

  //! This is to fix problems when generating C++
  // We are declaring the functions provided by Flex, so
  // that Bison generated code can call them.
  int yylex(void);
  void yyerror(const char *);
}

%union{
    std::string *word;
}

%token<word> CHAR SHORT INT LONG FLOAT DOUBLE VOID
%token  VARIABLE NUMBER CHAR_ASSIGN STRING_ASSIGN
%token IF ELSE WHILE FOR RETURN
%token ';' '{' '}' '(' ')' ',' ':' '+' '-' '/' '*' '[' ']' '|' '&' '^' '=' '?' '!' '>' '<'

%type <word> type_specifier variable_init declaration_statement assignment_specifier assigners
%start declaration_statement
%%

type_specifier
    : CHAR              { $$ = new std::string("char"); }
    | SHORT             { $$ = new std::string("short"); }
    | INT               { $$ = new std::string("int"); }
    | LONG              { $$ = new std::string("long"); }
    | FLOAT             { $$ = new std::string("float"); }
    | DOUBLE            { $$ = new std::string("double"); }
    | VOID              { $$ = new std::string("void"); }

    // tested, can be assigned to kind so it can be _kind = variable name, use typedef enum to assign these variables

assignment_specifier
    : variable_init '=' assigners                   { /* output should be "type" which goes to kind, the value which is stored in assigners (NULL if not)  */ }
    | variable_init                                 { /* output should be "type" which goes to kind, the value which is stored in assigners (NULL if not)  */ }
    | variable_init ',' assignment_specifier        { /* output should be "type" which goes to kind, the value which is stored in assigners (NULL if not). For this one, you might need to do a recursion for the last one.  */ }

variable_init
    : VARIABLE                                      { $$ = $1; }

    // tests, can be the key value pair in context.bindings

assigners
    : NUMBER                { $$ = $1; }
    | CHAR_ASSIGN           { $$ = $1; }
    | STRING_ASSIGN         { $$ = $1; }

/* $$ = new Assign($1, "number"); ,  $$ = new Assign($1, "char"); , $$ = new Assign($1, "string");*/

declaration_statement
    : type_specifier assignment_specifier           {  }

/* this is like a bigger statement. Can use the assignment specifier class talked about below over here.  */

/* result should be $$ = new Declaration($1); for the first one and $$ = new Declaration($1, $2); for the second one. We can have two constructors for the classes */

return_statement
    : RETURN
    | RETURN '(' evaluation_statement ')'
    | RETURN '!''(' evaluation_statement ')'
    | RETURN variable_init
    | RETURN assigners

/* need class for return statement, can do $$ = new Return(); , $$ = new Return($1); */

evaluation_statement
    : variable_init operators variable_init
    | variable_init operators assigners
    | variable_init operators

/* $$ = new conditionCheck($2, $1, $3); */

loop_statement
    : WHILE '(' evaluation_statement ')' '{' statement_list '}'
    | FOR '(' declaration_statement evaluation_statement ')' '{' statement_list '}'

/* can transform a for loop into a while loop so we only have to code while. */

conditional_statement
    : IF '(' evaluation_statement ')' '{' statement_list '}'
    | conditional_statement ELSE conditional_statement
    | conditional_statement ELSE '{' statement_list '}'

operators
    : binary_operators
    | increment_operators
    | '>'
    | '<'
    | '!'
    | '|'
    | '&'
    | '^'

binary_operators
    : '|''|'
    | '&''&'
    | '=''='
    | '!''='


/* $$ = new std::string($1); */

increment_operators
    : '+''+'
    | '-''-'

statement_list
    : statement statement_list              {}
    | statement

statement
    : return_statement ';'
    | loop_statement ';'
    | conditional_statement ';'
    | declaration_statement ';'

/* $$ = new statementHandler($1, "return") */

function
    : type_specifier variable_init '(' ')' '{' statement_list '}'
    | type_specifier variable_init '(' arguments ')' '{' statement_list '}'

arguments
    : type_specifier variable_init
    | arguments ',' type_specifier variable_init

%%

