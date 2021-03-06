/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void printToken( TokenType token, const char* tokenString )
{ switch (token)
  { case IF:
    case ELSE:
    case WHILE:
    case RETURN:
    case INT:
    case VOID:
      fprintf(listing,
         "reserved word: %s\n",tokenString);
      break;
    case ASSIGN: fprintf(listing,"=\n"); break;
    case EQ: fprintf(listing,"==\n"); break;
    case NE: fprintf(listing,"!=\n"); break;
    case LT: fprintf(listing,"<\n"); break;
    case LE: fprintf(listing,"<=\n"); break;
    case GT: fprintf(listing,">\n"); break;
    case GE: fprintf(listing,">=\n"); break;
    case LPAREN: fprintf(listing,"(\n"); break;
    case RPAREN: fprintf(listing,")\n"); break;
    case LBRACE: fprintf(listing,"[\n"); break;
    case RBRACE: fprintf(listing,"]\n"); break;
    case LCURLY: fprintf(listing,"{\n"); break;
    case RCURLY: fprintf(listing,"}\n"); break;
    case SEMI: fprintf(listing,";\n"); break;
    case COMMA: fprintf(listing,",\n"); break;
    case PLUS: fprintf(listing,"+\n"); break;
    case MINUS: fprintf(listing,"-\n"); break;
    case TIMES: fprintf(listing,"*\n"); break;
    case OVER: fprintf(listing,"/\n"); break;
    case ENDFILE: fprintf(listing,"EOF\n"); break;
    case NUM:
      fprintf(listing,
          "NUM, val= %s\n",tokenString);
      break;
    case ID:
      fprintf(listing,
          "ID, name= %s\n",tokenString);
      break;
    case ERROR:
      fprintf(listing,
          "ERROR: %s\n",tokenString);
      break;
    default: /* should never happen */
      fprintf(listing,"Unknown token: %d\n",token);
  }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newDeclNode creates a new declaration
 * node for syntax tree construction
 */
TreeNode * newDeclNode(DeclKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = DeclK;
    t->kind.decl = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode * newExpNode(ExpKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;
  }
  return t;
}

/* Function newDeclNode creates a new parameter
 * node for syntax tree construction
 */
TreeNode * newParamNode(ParamKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ParamK;
    t->kind.param = kind;
    t->lineno = lineno;
    t->type = Void;
  }
  return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString(char * s)
{ int n;
  char * t;
  if (s==NULL) return NULL;
  n = strlen(s)+1;
  t = malloc(n);
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else strcpy(t,s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{ int i;
  for (i=0;i<indentno;i++)
    fprintf(listing," ");
}

/* getType print types of funcions and variables */
char * getType(TreeNode* tree) {
  static char int_[]="int";
  static char void_[]="void";
  static char error_[]="ERROR";

  if (tree->child[0]!=NULL) {
    switch (tree->child[0]->type) {
      case Integer:
        return int_;
      case Void:
        return void_;
      default:
        return error_;
    }
  } else {
    switch (tree->type) {
      case Integer:
        return int_;
      case Void:
        return void_;
      default:
        return error_;
    }
  }
}

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree( TreeNode * tree )
{ int i;
  INDENT;
  while (tree != NULL) {
    if (tree->nodekind==StmtK)
    { printSpaces();
      switch (tree->kind.stmt) {
        case CompK:
          fprintf(listing,"Compound statement :\n");
          break;
        case SelK:
          if(tree->child[2] != NULL)
            fprintf(listing,"If (condition) (body) (else)\n");
          else
            fprintf(listing,"If (condition) (body)\n");
          break;
        case IterK:
          fprintf(listing,"While (condition)\n");
          break;
        case RetK:
          fprintf(listing,"Return :\n");
          break;
        default:
          fprintf(listing,"Unknown ExpNode kind\n");
          break;
      }
    }
    else if (tree->nodekind==DeclK)
    { printSpaces();
      switch (tree->kind.decl) {
        case VarK:
          fprintf(listing,"Variable declaration, name : %s,",tree->attr.name);
          switch(tree->type) {
            case Integer:
              fprintf(listing," type : int\n");
              break;
            case Void:
              fprintf(listing," type : void\n");
              break;
            default:
              fprintf(listing,"\n");
              break;
          }
          break;
        case VarArrK:
          fprintf(listing,"Variable (Array) declaration, name : %s,",tree->attr.array.name);
          switch(tree->type) {
            case IntegerArray:
              fprintf(listing," type : int array,");
              break;
            case VoidArray:
              fprintf(listing," type : void array,");
              break;
            default:
              fprintf(listing,"\n");
              break;
          }
          fprintf(listing," size : %d\n", tree->attr.array.size);
          break;
        case FunK:
          fprintf(listing,"Function declaration, name : %s, return",tree->attr.name);
          switch(tree->type) {
            case Integer:
              fprintf(listing," type : int\n");
              break;
            case Void:
              fprintf(listing," type : void\n");
              break;
            default:
              fprintf(listing,"\n");
              break;
          }
          break;
        default:
          fprintf(listing,"Unknown DeclNode kind\n");
          break;
      }
    }
    else if (tree->nodekind==ExpK)
    { printSpaces();
      switch (tree->kind.exp) {
        case AssignK:
          fprintf(listing,"Assign : (destination) (source)\n");
          break;
        case RelopK:
        case OpK:
          fprintf(listing,"Op : ");
          printToken(tree->attr.op,"\0");
          break;
        case ConstK:
          fprintf(listing,"Const : %d\n",tree->attr.val);
          break;
        case IdK:
          fprintf(listing,"Id : %s\n",tree->attr.name);
          break;
        case IdArrK:
          fprintf(listing,"IdArr : %s, with array index below\n",tree->attr.name);
          break;
        case CallK:
          fprintf(listing,"Call, name : %s, with arguments below\n",tree->attr.name);
          break;
        default:
          fprintf(listing,"Unknown ExpNode kind\n");
          break;
      }
    }
    else if (tree->nodekind==ParamK)
    { printSpaces();
      switch (tree->kind.param) {
        case ArrParamK:
        case SingleParamK:
          if(tree->attr.name != NULL) {
            fprintf(listing,"Parameter, name : %s,",tree->attr.name);
            switch(tree->type) {
              case Integer:
                fprintf(listing," type : int (single)\n");
                break;
              case IntegerArray:
                fprintf(listing," type : int (array)\n");
                break;
              case Void:
                fprintf(listing," type : void (single)\n");
                break;
              case VoidArray:
                fprintf(listing," type : void (array)\n");
                break;
              default:
                fprintf(listing,"\n");
                break;
            }
          }
          else {
            fprintf(listing,"Parameter, name : (NULL),");
            switch(tree->type) {
              case Integer:
                fprintf(listing," type : int (single)\n");
                break;
              case IntegerArray:
                fprintf(listing," type : int (array)\n");
                break;
              case Void:
                fprintf(listing," type : void (single)\n");
                break;
              case VoidArray:
                fprintf(listing," type : void (array)\n");
                break;
              default:
                fprintf(listing,"\n");
                break;
            }
          }
          break;
        default:
          fprintf(listing,"Unknown ParamNode kind\n");
          break;
      }
    }
    else fprintf(listing,"Unknown node kind\n");
    for (i=0;i<MAXCHILDREN;i++)
         printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
