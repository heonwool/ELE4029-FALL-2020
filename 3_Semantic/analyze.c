/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

static ScopeList globalScope = NULL;
static char * funcName;
static int preserveLastScope = FALSE;

/* counter for variable memory locations */
static int location = 0;
static int funcReturn = 0;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* Procedure insertBuiltinFunc inserts 
 * Builtin functions such as input and output 
 * into the symbol table 
 */
static void insertBuiltinFunc(void)
{ TreeNode *func;
  TreeNode *param;
  TreeNode *compStmt;

  param = newParamNode(SingleParamK);
  param->attr.name = NULL;
  param->type = Void;

  compStmt = newStmtNode(CompK);
  compStmt->child[0] = NULL; // No local variables
  compStmt->child[1] = NULL; // No stmt

  func = newDeclNode(FunK);
  func->lineno = 0;
  func->attr.name = "input";
  func->type = Integer;
  func->child[0] = param;
  func->child[1] = compStmt;
  
  st_insert("input", -1, location, func);

  param = newParamNode(SingleParamK);
  param->attr.name = "arg";
  param->type = Integer;

  compStmt = newStmtNode(CompK);
  compStmt->child[0] = NULL; // No local variables
  compStmt->child[1] = NULL; // No stmt

  func = newDeclNode(FunK);
  func->lineno = 0;
  func->attr.name = "output";
  func->type = Void;
  func->child[0] = param;
  func->child[1] = compStmt;
  
  st_insert("output", -1, location, func);
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

static void symbolError(TreeNode * t, char * message)
{ fprintf(listing,"Symbol error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          if (preserveLastScope)
            preserveLastScope = FALSE;
          else
          { ScopeList scope = scCreate(funcName);
            scPush(scope);
            location++;
          }
          t->attr.scope = scTop();
        default:
          break;
      }
      break;
    case DeclK:
      switch (t->kind.decl)
      { case FunK:
          funcName = t->attr.name;
          if (st_lookup_sc(funcName))
          { /* already in table, so it's an error */
            symbolError(t,"function already declared for current scope");
            break;
          }

          st_insert(funcName,t->lineno,location,t);
          scPush(scCreate(funcName));
          location++;
          preserveLastScope = TRUE;

          break;
        case VarK:
        case VarArrK:
          { char *name;
            if (t->type == Void || t->type == VoidArray) {
              symbolError(t,"variable should have non-void type");
              break;
            }
              
            if (t->kind.decl == VarK) name = t->attr.name;
            else name = t->attr.array.name;

            if (!st_lookup_sc(name))
              st_insert(name,t->lineno,location,t);
            else
              symbolError(t,"symbol already declared for current scope");
          }
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case IdK:
        case IdArrK:
        case CallK:
          if (st_lookup(t->attr.name) == -1)
          { /* not yet in table, so it's an error */
            symbolError(t,"using undeclared symbol");
            t->nodekind = DeclK;
            t->type = TypeError;
            st_insert(t->attr.name,t->lineno,location,t);
          }  
          else
          /* already in table, so ignore location, 
             add line number of use only */ 
            st_add_lineno(t->attr.name,t->lineno);
          break;
        default:
          break;
      }
      break;
    case ParamK:
      if (t->attr.name == NULL)
      { if(t->type == Void) // Void argument in Function Declaration, continue.
          break;
        else
          symbolError(t,"parameters except (void) must have name");
      }
      else if (st_lookup(t->attr.name) == -1)
        st_insert(t->attr.name,t->lineno,location,t);
      else
        symbolError(t,"symbol already declared for current scope");
      break;
    default:
      break;
  }
}

static void afterInsertNode( TreeNode *t )
{ if(t->nodekind == StmtK && t->kind.stmt == CompK)
  { scPop();
    location--;
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ globalScope = scCreate("<GLOBAL>");
  location = 0;
  
  scPush(globalScope);
  insertBuiltinFunc();
  traverse(syntaxTree,insertNode,afterInsertNode);
  scPop();

  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

static void beforeCheckNode(TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          scPush(t->attr.scope);
          break;
        default:
          break;
      }
      break;
    case DeclK:
      switch (t->kind.decl)
      { case FunK:
          funcName = t->attr.name;
          if(t->type == Void) funcReturn = 1;
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case ConstK:
          t->type = Integer;
          break;
        case IdK:
        case IdArrK:
          { char *symbolName = t->attr.name;
            BucketList bucket = st_lookup_bucket(symbolName);
            TreeNode *symbolDecl = NULL;
            
            if (bucket == NULL) 
              break;
            
            symbolDecl = bucket->treeNode;
            
            if (t->kind.exp == IdArrK)
            { if (symbolDecl->kind.decl != VarArrK && symbolDecl->kind.param != ArrParamK)
                typeError(t,"expected array symbol");
              else if (t->child[0]->type != Integer)
                switch(t->child[0]->type)
                { case IntegerArray:
                    typeError(t,"expected integer type index, got IntegerArray type index");
                    break;
                  case Void:
                    typeError(t,"expected integer type index, got Void type index");
                    break;
                  case VoidArray:
                    typeError(t,"expected integer type index, got VoidArray type index");
                    break;
                  default:
                    break;
                }
              else
                t->type = Integer;
            }
            else
              t->type = symbolDecl->type;
          }
          break;
        case CallK:
          { char *callFuncName = t->attr.name;
            TreeNode * funcDecl = st_lookup_bucket(callFuncName)->treeNode;
            TreeNode *arg;
            TreeNode *param;
            
            if (funcDecl == NULL)
              break;

            arg = t->child[0];
            param = funcDecl->child[0];
            
            if (funcDecl->kind.decl != FunK)
            { typeError(t,"expected function symbol");
              break;
            }
            
            if (arg == NULL)
            { if (param->attr.name == NULL && param->type == Void)
                ;
              else if (param != NULL)
                typeError(t,"invalid function call (# of arguments does not match)");
            }
            
            while (arg != NULL)
            { if (param == NULL)
                typeError(t,"invalid function call (# of arguments does not match)");
              else if (arg->type != param->type)
                typeError(t,"invalid function call (argument type mismatched)");
              else if (arg->type == Void)
                typeError(arg,"void value cannot be passed as an argument");
              else
              { arg = arg->sibling;
                param = param->sibling;
                continue;
              }
              break;
            }
            
            t->type = funcDecl->type;
          }
          break;
        case AssignK:
          if (t->child[0]->type != Integer || t->child[1]->type != Integer)
            typeError(t->child[0],"type conflict in assignment");
          else
            t->type = t->child[0]->type;
          break;
        case RelopK:
          if (t->child[0]->type == Integer && t->child[1]->type == Integer)
            t->type = Integer;
          else
            typeError(t,"operand of Relop should be Integer type");
          break;
        case OpK:
          if (t->child[0]->type == Integer && t->child[1]->type == Integer)
            t->type = Integer;
          else
            typeError(t,"operand of Op should be Integer type");
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          scPop();
          break;
        case SelK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0], "invalid expression (if-condition must be Integer type)");
          break;
        case IterK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0], "invalid expression (while-condition must be Integer type)");
          break;
        case RetK:
          { const TreeNode * funcDecl = st_lookup_bucket(funcName)->treeNode;
            const ExpType funcType = funcDecl->type;
            const TreeNode * expr = t->child[0];
            
            if (funcType == Void)
            { if (expr != NULL && expr->type != Void)
              { typeError(t,"invalid return type (non-void return value in void type function)");
                funcReturn = 0;
              }
              else {
                funcReturn = 1;
              }
            } 
            else if (funcType == Integer) 
            { if (expr == NULL || expr->type != Integer)
              { typeError(t,"invalid return type (return value should be Integer)");
                funcReturn = 0;
              }
              else {
                funcReturn = 1;
              }
            }

            else {
              funcReturn = 0;
            }
          }
          break;
        default:
          break;
      }
      break;
    case DeclK:
      switch (t->kind.decl)
      { case VarK:
        case VarArrK:
          if (t->type == Void || t->type == VoidArray)
            typeError(t,"declaration of void or void array type variable is invalid");
          break;
        case FunK:
          if (funcReturn != 1) 
          { funcReturn = 0;
            typeError(t, "return statement is missing or not properly stated in this funciton");
          } 
          else
          { funcReturn = 0;
          }
          break;
        default:
          break;
      }
      break;
    case ParamK:
      switch (t->kind.param)
      { case SingleParamK:
          if(t->type == Void && t->sibling != NULL)
            typeError(t,"Void type Parameter is invalid");
          break;
        case ArrParamK:
          if(t->type == VoidArray)
            typeError(t,"Void Array type parameter is invalid");
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ scPush(globalScope);
  traverse(syntaxTree,beforeCheckNode,checkNode);
  scPop();
}
