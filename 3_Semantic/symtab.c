/********************************************************/
/* File: symtab.c                                       */
/* Symbol table implementation for the C-MINUS compiler */
/* (allows multiple symbol tables)                      */
/* Symbol table is implemented as a chained             */
/* hash table                                           */
/* Compiler Construction: Principles and Practice       */
/* Kenneth C. Louden                                    */
/********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* Define the LIMIT of scopes */
#define MAX_SCOPE 1000

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

static ScopeList scopes[MAX_SCOPE];
static int scopesIdx = 0;

static ScopeList stack[MAX_SCOPE];
static int stackIdx = 0;

/* Miscellaneous functions for ScopeList */
ScopeList scCreate( char * scopeName )
{ ScopeList newScope;

  newScope = (ScopeList) malloc(sizeof(struct ScopeListRec));
  newScope->scopeName = scopeName;
  newScope->nestedLevel = stackIdx;
  newScope->parent = scTop();

  scopes[scopesIdx++] = newScope;

  return newScope;
}

ScopeList scTop( void ) 
{ return stack[stackIdx - 1];
}

void scPush( ScopeList scope )
{ stack[stackIdx++] = scope;
}

void scPop( void )
{ --stackIdx;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, TreeNode * treeNode )
{ int h = hash(name);

  ScopeList top = scTop();
  BucketList l = top->hashTable[h];

  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;

  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->treeNode = treeNode;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = top->hashTable[h];
    top->hashTable[h] = l; }
  else /* found in table, so just add line number */
  { // ERROR!
    printf("Error occured in st_insert()\n");
  }
} /* st_insert */

BucketList st_lookup_bucket( char * name )
{ int h = hash(name);
  ScopeList sc = scTop();

  while(sc)
  { BucketList l = sc->hashTable[h];

    while ((l != NULL) && (strcmp(name,l->name) != 0))
      l = l->next;

    if (l != NULL) return l;
    sc = sc->parent;
  }
  return NULL;
}

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup( char * name )
{ BucketList l = st_lookup_bucket(name);

  if (l != NULL)
    return l->memloc;

  return -1;
}

int st_lookup_sc( char * name )
{ int h = hash(name);
  ScopeList sc = scTop();

  if (sc)
  { BucketList l = sc->hashTable[h];

    while ((l != NULL) && (strcmp(name,l->name) != 0))
      l = l->next;

    if (l != NULL) 
      return TRUE;
  }
  return FALSE;
}

int st_add_lineno( char * name, int lineno )
{ BucketList bl = st_lookup_bucket(name);
  LineList ll = bl->lines;
  
  while (ll->next != NULL)
    ll = ll->next;

  ll->next = (LineList) malloc(sizeof(struct LineListRec));
  ll->next->lineno = lineno;
  ll->next->next = NULL;
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTabLine(FILE * listing, BucketList *hashTable)
{ int i;
  
  for (i=0;i<SIZE;++i)
  { if (hashTable[i] != NULL)
    { BucketList bl = hashTable[i];
      TreeNode * node = bl->treeNode;

      while (bl != NULL)
      { LineList ll = bl->lines;
        
        fprintf(listing,"%-13s ",bl->name);

        switch (node->nodekind) {
        case DeclK:
          switch (node->kind.decl) {
            case VarK:
              fprintf(listing,"Variable      ");
              break;
            case VarArrK:
              fprintf(listing,"Array Var.    ");
              break;
            case FunK:
              fprintf(listing,"Function      ");
              break;
            default:
              fprintf(listing,"Unknown       ");
              break;
          }
          break;
        case ParamK:
          switch (node->kind.param) {
            case SingleParamK:
              fprintf(listing,"Variable      ");
              break;
            case ArrParamK:
              fprintf(listing,"Array Var.    ");
              break;
            default:
              fprintf(listing,"Unknown       ");
              break;
          }
          break;
        default:
          break;
        }

        switch (node->type) {
        case Void:
        case VoidArray:
          fprintf(listing,"Void        ");
          break;
        case Integer:
        case IntegerArray:
          fprintf(listing,"Integer     ");
          break;
        case TypeError:
          fprintf(listing,"TypeError   ");

        default:
          break;
        }

        while (ll != NULL)
        { fprintf(listing,"%3d ",ll->lineno);
          ll = ll->next;
        }

        fprintf(listing,"\n");
        bl = bl->next;
      }
    }
  }
}

void printSymTab(FILE * listing)
{ int i;
  for (i=0;i<scopesIdx;++i)
  { ScopeList scope = scopes[i];
    BucketList * hashTable = scope->hashTable;

    if (i == 0)
      fprintf(listing,"GLOBAL scope ");
    else
      fprintf(listing,"Function name: %s ", scope->scopeName);

    fprintf(listing,"(nested level: %d)\n", scope->nestedLevel);

    fprintf(listing,"Symbol Name   Symbol Type   Data Type     Line Numbers\n");
    fprintf(listing,"------------  ------------  ------------  ------------\n");

    printSymTabLine(listing, hashTable);

    fprintf(listing,"\n");
  }
} /* printSymTab */
