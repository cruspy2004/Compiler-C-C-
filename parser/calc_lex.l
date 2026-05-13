%{
/*
 * calc_lex.l  —  shared lexer for calculator parsers (Modules 2 & 3).
 * Produces tokens consumed by postfix.y / prefix.y / infix.y.
 */
#include <stdlib.h>
#include <string.h>
/* infix.tab.h is included by each .y file's generated .tab.h */
#include "infix.tab.h"
%}

%option yylineno
%option noyywrap

DIGIT  [0-9]
ALPHA  [a-zA-Z_]
ALNUM  [a-zA-Z0-9_]

%%

{DIGIT}+\.{DIGIT}+  { yylval.fval = strtod(yytext, NULL);  return FLOAT_LIT; }
{DIGIT}+            { yylval.fval = (double)atoi(yytext);   return INT_LIT;   }

"log"   { return LOG; }
"exp"   { return EXP; }

{ALPHA}{ALNUM}* { yylval.str = strdup(yytext); return IDENT; }

"+"  { return '+'; }
"-"  { return '-'; }
"*"  { return '*'; }
"/"  { return '/'; }
"^"  { return '^'; }
"("  { return '('; }
")"  { return ')'; }

[ \t\r]+  { /* discard spaces, tabs, carriage returns */ }
\n        { return '\n'; }   /* newline terminates an expression */

. { fprintf(stderr, "Lexical error at line %d: unexpected '%s'\n", yylineno, yytext); }

%%
