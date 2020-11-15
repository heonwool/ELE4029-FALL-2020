/****************************************************/
/* File: cminus.l                                   */
/* Lex specification for C-MINUS                    */
/* Compiler Construction: Principles and Practice   */
/* Original Author: Kenneth C. Louden               */
/* Revised: Jeongwoo Son @ Hanyang Univ (ELE4029)   */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
%}

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \n
whitespace  [ \t]+

%%

"if"            {return IF;}
"else"          {return ELSE;}
"while"         {return WHILE;}
"return"        {return RETURN;}
"int"           {return INT;}
"void"          {return VOID;}
"then"          {return THEN;}   /* discarded */
"end"           {return END;}    /* discarded */
"repeat"        {return REPEAT;} /* discarded */
"until"         {return UNTIL;}  /* discarded */
"read"          {return READ;}   /* discarded */
"write"         {return WRITE;}  /* discarded */
"="             {return ASSIGN;}
"=="            {return EQ;}
"!="            {return NE;}
"<"             {return LT;}
"<="            {return LE;}
">"             {return GT;}
">="            {return GE;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"/"             {return OVER;}
"("             {return LPAREN;}
")"             {return RPAREN;}
"["             {return LBRACE;}
"]"             {return RBRACE;}
"{"             {return LCURLY;}
"}"             {return RCURLY;}
";"             {return SEMI;}
","             {return COMMA;}
{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}
"/*"            { char b, c;
                  b = 0;
                  do
                  { c = input();
                    if (c == EOF) break;
                    else if (c == '\n') lineno++;
                    else if (b == '*' && c == '/') break;
                    b = c;
                  } while (TRUE);
                }
.               {return ERROR;}

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}

