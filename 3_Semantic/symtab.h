/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the C-MINUS compiler  */
/* (allows mulitiple symbol tables)                 */
/* Compiler Construction: Principles and Practice   */
/* Original Author: Kenneth C. Louden               */
/* Revised: Jeongwoo Son @ Hanyang Univ (ELE4029)   */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* The list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec
  { int lineno;
    struct LineListRec * next;
  } * LineList;
 
/* The record in the bucket lists for
 * each variable, including name, 
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
  { char * name;
    TreeNode *treeNode;
    LineList lines;
    int memloc; /* memory location for variable */
    struct BucketListRec * next;
  } * BucketList;

/* The record for each scope (including name, 
 * its bucket, and parent scope)
 */
typedef struct ScopeListRec
  { char * scopeName;
    int nestedLevel;
    BucketList hashTable[SIZE];
    struct ScopeListRec * parent;
    struct ScopeListRec * next;
  } * ScopeList;

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, TreeNode * treeNode );

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */

int st_lookup( char * name );
BucketList st_lookup_bucket( char * name );
int st_lookup_sc( char * name );
int st_add_lineno( char * name, int lineno );

/* Miscellaneous functions for ScopeList */
ScopeList scCreate( char * scopeName );
ScopeList scTop( void );
void scPush( ScopeList scope );
void scPop( void );

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);

#endif
