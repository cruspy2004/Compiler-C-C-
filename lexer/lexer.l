%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/token.h"

/* When used standalone (not with Bison), print token info directly. */
#ifndef BISON_MODE
#  define RETURN_TOK(t) do { printf("TOKEN: %-15s  TEXT: %s\n", tok_name(t), yytext); } while(0)
#else
#  include "infix.tab.h"
#  define RETURN_TOK(t) return (t)
#endif

static const char *tok_name(int t);
%}

%option yylineno
%option noyywrap

DIGIT   [0-9]
ALPHA   [a-zA-Z_]
ALNUM   [a-zA-Z0-9_]

%%

"int"           { RETURN_TOK(TOK_INT);       }
"float"         { RETURN_TOK(TOK_FLOAT);     }
"if"            { RETURN_TOK(TOK_IF);        }
"else"          { RETURN_TOK(TOK_ELSE);      }
"while"         { RETURN_TOK(TOK_WHILE);     }
"return"        { RETURN_TOK(TOK_RETURN);    }
"then"          { RETURN_TOK(TOK_THEN);      }
"begin"         { RETURN_TOK(TOK_BEGIN);     }
"end"           { RETURN_TOK(TOK_END);       }
"procedure"     { RETURN_TOK(TOK_PROCEDURE); }
"function"      { RETURN_TOK(TOK_FUNCTION);  }

{DIGIT}+\.{DIGIT}+ {
#ifdef BISON_MODE
    yylval.fval = strtod(yytext, NULL);
#endif
    RETURN_TOK(TOK_FLOAT_LIT);
}

{DIGIT}+ {
#ifdef BISON_MODE
    yylval.ival = (int)strtol(yytext, NULL, 10);
#endif
    RETURN_TOK(TOK_INT_LIT);
}

{ALPHA}{ALNUM}* {
#ifdef BISON_MODE
    yylval.str = strdup(yytext);
#endif
    RETURN_TOK(TOK_IDENT);
}

"=="    { RETURN_TOK(TOK_EQ);  }
"!="    { RETURN_TOK(TOK_NEQ); }
"+"     { RETURN_TOK('+'); }
"-"     { RETURN_TOK('-'); }
"*"     { RETURN_TOK('*'); }
"/"     { RETURN_TOK('/'); }
"^"     { RETURN_TOK('^'); }
"="     { RETURN_TOK('='); }
"("     { RETURN_TOK('('); }
")"     { RETURN_TOK(')'); }
"{"     { RETURN_TOK('{'); }
"}"     { RETURN_TOK('}'); }
";"     { RETURN_TOK(';'); }
","     { RETURN_TOK(','); }

"//"[^\n]*          { /* single-line comment, discard */ }
"/*"([^*]|\*[^/])*"*/" { /* multi-line comment, discard */ }

[ \t\r\n]+  { /* whitespace, discard */ }

. {
    fprintf(stderr, "Lexical error at line %d: unexpected character '%s'\n",
            yylineno, yytext);
}

%%

static const char *tok_name(int t) {
    switch (t) {
        case TOK_INT_LIT:    return "INT_LIT";
        case TOK_FLOAT_LIT:  return "FLOAT_LIT";
        case TOK_IDENT:      return "IDENT";
        case TOK_INT:        return "KW_int";
        case TOK_FLOAT:      return "KW_float";
        case TOK_IF:         return "KW_if";
        case TOK_ELSE:       return "KW_else";
        case TOK_WHILE:      return "KW_while";
        case TOK_RETURN:     return "KW_return";
        case TOK_THEN:       return "KW_then";
        case TOK_BEGIN:      return "KW_begin";
        case TOK_END:        return "KW_end";
        case TOK_PROCEDURE:  return "KW_procedure";
        case TOK_FUNCTION:   return "KW_function";
        case TOK_EQ:         return "EQ(==)";
        case TOK_NEQ:        return "NEQ(!=)";
        case '+':            return "PLUS";
        case '-':            return "MINUS";
        case '*':            return "STAR";
        case '/':            return "SLASH";
        case '^':            return "CARET";
        case '=':            return "ASSIGN";
        case '(':            return "LPAREN";
        case ')':            return "RPAREN";
        case '{':            return "LBRACE";
        case '}':            return "RBRACE";
        case ';':            return "SEMI";
        case ',':            return "COMMA";
        default:             return "UNKNOWN";
    }
}

/* Standalone main — only compiled without BISON_MODE */
#ifndef BISON_MODE
int main(int argc, char *argv[]) {
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) { perror(argv[1]); return 1; }
        yyin = f;
    }
    printf("%-20s  %s\n", "TOKEN", "TEXT");
    printf("%-20s  %s\n", "-----", "----");
    int tok;
    while ((tok = yylex()) != 0)
        ;   /* RETURN_TOK already printed */
    return 0;
}
#endif
