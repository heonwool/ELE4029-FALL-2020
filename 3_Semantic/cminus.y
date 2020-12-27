/****************************************************/
/* File: cminus.y                                   */
/* The C-MINUS Yacc/Bison specification file        */
/* Compiler Construction: Principles and Practice   */
/* Original Author: Kenneth C. Louden               */
/* Revised: Jeongwoo Son @ Hanyang Univ (ELE4029)   */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedNumber;
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex
int yyerror(char*); // added explicit declaration to ensure no warning message during compilation

%}

%token IFX
%token IF ELSE INT RETURN VOID WHILE
%token ID NUM
%token EQ NE LT LE GT GE SEMI
%token LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY
%token PLUS MINUS TIMES OVER COMMA ASSIGN
%token ERROR 

%nonassoc IFX
%nonassoc ELSE

%left PLUS MINUS
%left TIMES OVER COMMA
%right ASSIGN

%% /* Grammar for C-MINUS */

program             : declaration_list
                         { savedTree = $1; } 
                    ;
declaration_list    : declaration declaration_list
                         { YYSTYPE t = $1;
                           if (t != NULL)
                           { while (t->sibling != NULL)
                                t = t->sibling;
                             t->sibling = $2;
                             $$ = $1; }
                             else $$ = $2;
                         }
                    | declaration  { $$ = $1; }
                    ;
declaration         : var_declaration  { $$ = $1;}
                    | fun_declaration  { $$ = $1;}
                    ;
inputName           : ID
                         { savedName = copyString(tokenString); }
                    ;
inputNumber         : NUM
                         { savedNumber = atoi(tokenString); }
                    ;
var_declaration     : INT inputName SEMI
                         { $$ = newDeclNode(VarK);
                           $$->lineno = lineno;
                           $$->attr.name = savedName;
                           $$->type = Integer;
                         }
                    | INT inputName LBRACE inputNumber RBRACE SEMI
                         { $$ = newDeclNode(VarArrK);
                           $$->lineno = lineno;
                           $$->attr.array.name = savedName;
                           $$->attr.array.size = savedNumber;
                           $$->type = IntegerArray;
                         }
                    | VOID inputName SEMI
                         { $$ = newDeclNode(VarK);
                           $$->lineno = lineno;
                           $$->attr.name = savedName;
                           $$->type = Void;
                         }
                    | VOID inputName LBRACE inputNumber RBRACE SEMI
                         { $$ = newDeclNode(VarArrK);
                           $$->lineno = lineno;
                           $$->attr.array.name = savedName;
                           $$->attr.array.size = savedNumber;
                           $$->type = VoidArray;
                         }
                    ;
fun_declaration     : INT inputName
                         { $$ = newDeclNode(FunK);
                           $$->lineno = lineno;
                           $$->attr.name = savedName;
                           $$->type = Integer;
                         }
                      LPAREN params RPAREN compound_stmt 
                         { $$ = $3;
                           $$->child[0] = $5; // Params
                           $$->child[1] = $7; // Body
                         }
                    | VOID inputName
                         { $$ = newDeclNode(FunK);
                           $$->lineno = lineno;
                           $$->attr.name = savedName;
                           $$->type = Void;
                         }
                      LPAREN params RPAREN compound_stmt 
                         { $$ = $3;
                           $$->child[0] = $5; // Params
                           $$->child[1] = $7; // Body
                         }
                    ;
params              : param_list  { $$ = $1; }
                    | VOID
                         { $$ = newParamNode(SingleParamK);
                           $$->type = Void;
                           $$->attr.name = NULL;
                         }
                    ;
param_list          : param_list COMMA param
                         { YYSTYPE t = $1;
                           if (t != NULL)
                           { while (t->sibling != NULL)
                                t = t->sibling;
                             t->sibling = $3;
                             $$ = $1; }
                             else $$ = $3;
                         }
                    | param  { $$ = $1; }
                    ;
param               : INT inputName
                         { $$ = newParamNode(SingleParamK);
                           $$->attr.name = savedName;
                           $$->type = Integer;
                         }
                    | INT inputName LBRACE RBRACE
                         { $$ = newParamNode(ArrParamK);
                           $$->attr.name = savedName;
                           $$->type = IntegerArray;
                         }
                    | VOID inputName
                         { $$ = newParamNode(SingleParamK);
                           $$->attr.name = savedName;
                           $$->type = Void;
                         }
                    | VOID inputName LBRACE RBRACE
                         { $$ = newParamNode(ArrParamK);
                           $$->attr.name = savedName;
                           $$->type = VoidArray;
                         }
                    ;
compound_stmt       : LCURLY local_declarations statement_list RCURLY
                         { $$ = newStmtNode(CompK);
                           $$->child[0] = $2; // Local variable declarations
                           $$->child[1] = $3; // Statements
                         }
                    ;
local_declarations  : local_declarations var_declaration
                         { YYSTYPE t = $1;
                           if (t != NULL)
                           { while (t->sibling != NULL)
                                t = t->sibling;
                             t->sibling = $2;
                             $$ = $1; }
                             else $$ = $2;
                         }
                    | /* empty */  { $$ = NULL; }
                    ;
statement_list      : statement_list statement
                         { YYSTYPE t = $1;
                           if (t != NULL)
                           { while (t->sibling != NULL)
                                t = t->sibling;
                             t->sibling = $2;
                             $$ = $1; }
                             else $$ = $2;
                         }
                    | /* empty */  { $$ = NULL; }
                    ;
statement           : expression_stmt  { $$ = $1; }
                    | compound_stmt  { $$ = $1; }
                    | selection_stmt  { $$ = $1; }
                    | iteration_stmt  { $$ = $1; }
                    | return_stmt  { $$ = $1; }
                    ;
expression_stmt     : expression SEMI  { $$ = $1; }
                    | SEMI  { $$ = NULL; }
                    ;
selection_stmt      : IF LPAREN expression RPAREN statement %prec IFX
                         { $$ = newStmtNode(SelK);
                           $$->child[0] = $3; // If-condition
                           $$->child[1] = $5; // When it satisfy the condition
                         }
                    | IF LPAREN expression RPAREN statement ELSE statement
                         { $$ = newStmtNode(SelK);
                           $$->child[0] = $3; // If-condition
                           $$->child[1] = $5; // When it satisfy the condition (Jump into IF)
                           $$->child[2] = $7; // When it does not satisfy the condition (Jump into ELSE)
                         }
                    ;
iteration_stmt      : WHILE LPAREN expression RPAREN statement
                         { $$ = newStmtNode(IterK);
                           $$->child[0] = $3; // While-condition
                           $$->child[1] = $5; // Statements
                         }
                    ;
return_stmt         : RETURN SEMI
                         { $$ = newStmtNode(RetK);
                           $$->type = Void; // Return NOTHING;
                           $$->child[0] = NULL;
                         }
                    | RETURN expression SEMI
                         { $$ = newStmtNode(RetK);
                           $$->child[0] = $2; // Return expression;
                         }
                    ;
expression          : var ASSIGN expression
                         { $$ = newExpNode(AssignK);
                           $$->child[0] = $1; // Variable
                           $$->child[1] = $3; // Expression
                         }
                    | simple_expression  { $$ = $1; }
                    ;
var                 : inputName
                         { $$ = newExpNode(IdK);
                           $$->attr.name = savedName;
                         }
                    | inputName
                         { $$ = newExpNode(IdArrK);
                           $$->attr.name = savedName;
                         } 
                      LBRACE expression RBRACE
                         { $$ = $2;
                           $$->child[0] = $4;
                         }
                    ;
simple_expression   : additive_expression EQ additive_expression
                         { $$ = newExpNode(RelopK);
                           $$->child[0] = $1; // Left expression
                           $$->child[1] = $3; // Right expression
                           $$->attr.op = EQ;
                         }
                    | additive_expression NE additive_expression
                         { $$ = newExpNode(RelopK);
                           $$->child[0] = $1; // Left expression
                           $$->child[1] = $3; // Right expression
                           $$->attr.op = NE;
                         }
                    | additive_expression LT additive_expression
                         { $$ = newExpNode(RelopK);
                           $$->child[0] = $1; // Left expression
                           $$->child[1] = $3; // Right expression
                           $$->attr.op = LT;
                         }
                    | additive_expression LE additive_expression
                         { $$ = newExpNode(RelopK);
                           $$->child[0] = $1; // Left expression
                           $$->child[1] = $3; // Right expression
                           $$->attr.op = LE;
                         }
                    | additive_expression GT additive_expression
                         { $$ = newExpNode(RelopK);
                           $$->child[0] = $1; // Left expression
                           $$->child[1] = $3; // Right expression
                           $$->attr.op = GT;
                         }
                    | additive_expression GE additive_expression
                         { $$ = newExpNode(RelopK);
                           $$->child[0] = $1; // Left expression
                           $$->child[1] = $3; // Right expression
                           $$->attr.op = GE;
                         }
                    | additive_expression  { $$ = $1; }
                    ;
additive_expression : additive_expression PLUS term
                         { $$ = newExpNode(OpK);
                           $$->child[0] = $1; // Left expression
                           $$->child[1] = $3; // Right expression
                           $$->attr.op = PLUS;
                         }
                    | additive_expression MINUS term
                         { $$ = newExpNode(OpK);
                           $$->child[0] = $1; // Left expression
                           $$->child[1] = $3; // Right expression
                           $$->attr.op = MINUS;
                         }
                    | term  { $$ = $1; }
                    ;
term                : term TIMES factor
                         { $$ = newExpNode(OpK);
                           $$->child[0] = $1; // Left expression
                           $$->child[1] = $3; // Right expression
                           $$->attr.op = TIMES;
                         }
                    | term OVER factor
                         { $$ = newExpNode(OpK);
                           $$->child[0] = $1; // Left expression
                           $$->child[1] = $3; // Right expression
                           $$->attr.op = OVER;
                         }
                    | factor  { $$ = $1; }
                    ;
factor              : LPAREN expression RPAREN  { $$ = $2; }
                    | var  { $$ = $1; }
                    | call  { $$ = $1; }
                    | NUM
                         { $$ = newExpNode(ConstK);
                           $$->attr.val = atoi(tokenString);
                         }
                    ;
call                : inputName 
                         { $$ = newExpNode(CallK);
                           $$->attr.name = savedName;
                         }
                      LPAREN args RPAREN
                         { $$ = $2;
                           $$->child[0] = $4; // Arguments
                         }
                    ;
args                : arg_list  { $$ = $1; }
                    | /* empty */  { $$ = NULL; }
                    ;
arg_list            : arg_list COMMA expression
                         { YYSTYPE t = $1;
                           if (t != NULL)
                           { while (t->sibling != NULL)
                                t = t->sibling;
                             t->sibling = $3;
                             $$ = $1; }
                             else $$ = $3;
                         }
                    | expression  { $$ = $1; }
                    ;

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

