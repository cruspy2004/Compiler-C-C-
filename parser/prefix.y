%{
/*
 * prefix.y  —  Module 2: Prefix (Polish notation) expression parser.
 * Parses prefix expressions like "+ 4 8" and evaluates them.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../common/ast.h"

extern int  yylex(void);
extern int  yylineno;
void yyerror(const char *s);
%}

%code requires {
#include "../common/ast.h"
}

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
      program expr '\n'  {
            ast_print($2, 0);
            printf("Value: %g\n\n", $2->num_val);
        }
    | /* empty */
    ;

/*
 * Prefix grammar: operator appears BEFORE its two operands.
 *   expr -> num | '+' expr expr | '-' expr expr | '*' expr expr | '/' expr expr
 */
expr:
      INT_LIT   {
                    $$ = ast_new_node(NODE_FLOAT_LIT, yylineno);
                    $$->num_val = $1;
                }
    | FLOAT_LIT {
                    $$ = ast_new_node(NODE_FLOAT_LIT, yylineno);
                    $$->num_val = $1;
                }
    | '+' expr expr {
                    $$ = ast_new_node(NODE_BINOP, yylineno);
                    $$->str_val = strdup("+");
                    $$->left = $2; $$->right = $3;
                    $$->num_val = $2->num_val + $3->num_val;
                }
    | '-' expr expr {
                    $$ = ast_new_node(NODE_BINOP, yylineno);
                    $$->str_val = strdup("-");
                    $$->left = $2; $$->right = $3;
                    $$->num_val = $2->num_val - $3->num_val;
                }
    | '*' expr expr {
                    $$ = ast_new_node(NODE_BINOP, yylineno);
                    $$->str_val = strdup("*");
                    $$->left = $2; $$->right = $3;
                    $$->num_val = $2->num_val * $3->num_val;
                }
    | '/' expr expr {
                    $$ = ast_new_node(NODE_BINOP, yylineno);
                    $$->str_val = strdup("/");
                    $$->left = $2; $$->right = $3;
                    if ($3->num_val == 0) { yyerror("division by zero"); YYERROR; }
                    $$->num_val = $2->num_val / $3->num_val;
                }
    | '^' expr expr {
                    $$ = ast_new_node(NODE_BINOP, yylineno);
                    $$->str_val = strdup("^");
                    $$->left = $2; $$->right = $3;
                    $$->num_val = pow($2->num_val, $3->num_val);
                }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error at line %d: %s\n", yylineno, s);
}

int main(void) {
    printf("Prefix Calculator (enter prefix expressions, one per line):\n");
    return yyparse();
}
