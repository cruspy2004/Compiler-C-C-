%{
/*
 * postfix.y  —  Module 2: Postfix expression parser.
 * Reads a postfix expression and evaluates it, printing the parse tree.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../common/ast.h"

extern int  yylex(void);
extern int  yylineno;
void yyerror(const char *s);

/* Simple stack for postfix evaluation */
#define MAX_STACK 256
static double eval_stack[MAX_STACK];
static int    eval_top = -1;

static void epush(double v) { eval_stack[++eval_top] = v; }
static double epop(void)    { return eval_stack[eval_top--]; }

static ASTNode *node_stack[MAX_STACK];
static int      ntop = -1;
static void npush(ASTNode *n) { node_stack[++ntop] = n; }
static ASTNode *npop(void)    { return node_stack[ntop--]; }
%}

%union {
    double      fval;
    char       *str;
    ASTNode    *node;
}

%token <fval> INT_LIT FLOAT_LIT
%token <str>  IDENT
%token LOG EXP

%type <node> expr

%%

program:
      program expr '\n'  { ast_print($2, 0); printf("Value: %g\n\n", $2->num_val); }
    | /* empty */
    ;

expr:
      INT_LIT   {
                    $$ = ast_new_node(NODE_FLOAT_LIT, yylineno);
                    $$->num_val = $1;
                    epush($1);
                    npush($$);
                }
    | FLOAT_LIT {
                    $$ = ast_new_node(NODE_FLOAT_LIT, yylineno);
                    $$->num_val = $1;
                    epush($1);
                    npush($$);
                }
    | expr expr '+'  {
                    ASTNode *r = npop(), *l = npop();
                    $$ = ast_new_node(NODE_BINOP, yylineno);
                    $$->str_val = strdup("+");
                    $$->left = l; $$->right = r;
                    double b = epop(), a = epop();
                    $$->num_val = a + b;
                    epush($$->num_val);
                    npush($$);
                }
    | expr expr '-'  {
                    ASTNode *r = npop(), *l = npop();
                    $$ = ast_new_node(NODE_BINOP, yylineno);
                    $$->str_val = strdup("-");
                    $$->left = l; $$->right = r;
                    double b = epop(), a = epop();
                    $$->num_val = a - b;
                    epush($$->num_val);
                    npush($$);
                }
    | expr expr '*'  {
                    ASTNode *r = npop(), *l = npop();
                    $$ = ast_new_node(NODE_BINOP, yylineno);
                    $$->str_val = strdup("*");
                    $$->left = l; $$->right = r;
                    double b = epop(), a = epop();
                    $$->num_val = a * b;
                    epush($$->num_val);
                    npush($$);
                }
    | expr expr '/'  {
                    ASTNode *r = npop(), *l = npop();
                    $$ = ast_new_node(NODE_BINOP, yylineno);
                    $$->str_val = strdup("/");
                    $$->left = l; $$->right = r;
                    double b = epop(), a = epop();
                    if (b == 0) { yyerror("division by zero"); YYERROR; }
                    $$->num_val = a / b;
                    epush($$->num_val);
                    npush($$);
                }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error at line %d: %s\n", yylineno, s);
}

int main(void) {
    printf("Postfix Calculator (enter postfix expressions, one per line):\n");
    return yyparse();
}
