%{
/*
 * infix.y  —  Modules 2 & 3: Infix expression parser with full precedence,
 *             right-associative exponentiation, and built-in log/exp.
 *
 * Grammar implemented (from PRD FR-3):
 *   E  -> E + T | E - T | T
 *   T  -> T * F | T / F | F
 *   F  -> B ^ F          (right-assoc via %right '^')
 *   B  -> ( E ) | id | num | log( E ) | exp( E )
 *
 * Debug mode: compile with -DYYDEBUG and set yydebug=1 in main().
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../common/ast.h"

extern int  yylex(void);
extern int  yylineno;
void yyerror(const char *s);

/* Tree traversal helpers. */
static void print_preorder (ASTNode *n);
static void print_inorder  (ASTNode *n);
static void print_postorder(ASTNode *n);
static double eval(ASTNode *n);

#ifdef YYDEBUG
int yydebug = 1;
#endif
%}

/* Make ast.h available in the generated infix.tab.h so calc_lex.l can
   include infix.tab.h without a forward-declaration problem. */
%code requires {
#include "../common/ast.h"
}

%union {
    double      fval;
    char       *str;
    ASTNode    *node;
}

%token <fval>  INT_LIT FLOAT_LIT
%token <str>   IDENT
%token LOG EXP

%type <node> expr term factor base

/* Precedence table — lowest to highest */
%left  '+' '-'
%left  '*' '/'
%right '^'
%right UMINUS

%%

program:
      program expr '\n'  {
            printf("--- Parse Tree (inorder) : ");
            print_inorder($2);  printf("\n");
            printf("--- Prefix notation      : ");
            print_preorder($2); printf("\n");
            printf("--- Postfix notation     : ");
            print_postorder($2); printf("\n");
            printf("--- Value                : %g\n\n", eval($2));
        }
    | /* empty */
    ;

expr:
      expr '+' term  {
            $$ = ast_new_node(NODE_BINOP, yylineno);
            $$->str_val = strdup("+");
            $$->left = $1; $$->right = $3;
        }
    | expr '-' term  {
            $$ = ast_new_node(NODE_BINOP, yylineno);
            $$->str_val = strdup("-");
            $$->left = $1; $$->right = $3;
        }
    | term  { $$ = $1; }
    ;

term:
      term '*' factor  {
            $$ = ast_new_node(NODE_BINOP, yylineno);
            $$->str_val = strdup("*");
            $$->left = $1; $$->right = $3;
        }
    | term '/' factor  {
            $$ = ast_new_node(NODE_BINOP, yylineno);
            $$->str_val = strdup("/");
            $$->left = $1; $$->right = $3;
        }
    | factor  { $$ = $1; }
    ;

factor:
      base '^' factor  {   /* right-associative exponentiation */
            $$ = ast_new_node(NODE_BINOP, yylineno);
            $$->str_val = strdup("^");
            $$->left = $1; $$->right = $3;
        }
    | base  { $$ = $1; }
    | '-' factor %prec UMINUS  {
            $$ = ast_new_node(NODE_UNOP, yylineno);
            $$->str_val = strdup("-");
            $$->left = $2;
        }
    ;

base:
      '(' expr ')'  { $$ = $2; }
    | INT_LIT    {
            $$ = ast_new_node(NODE_FLOAT_LIT, yylineno);
            $$->num_val = $1;
        }
    | FLOAT_LIT  {
            $$ = ast_new_node(NODE_FLOAT_LIT, yylineno);
            $$->num_val = $1;
        }
    | IDENT      {
            $$ = ast_new_node(NODE_IDENT, yylineno);
            $$->str_val = $1;
        }
    | LOG '(' expr ')' {
            $$ = ast_new_node(NODE_LOG, yylineno);
            $$->left = $3;
        }
    | EXP '(' expr ')' {
            $$ = ast_new_node(NODE_EXP, yylineno);
            $$->left = $3;
        }
    ;

%%

/* ---------- tree traversals ---------- */

static void print_preorder(ASTNode *n) {
    if (!n) return;
    if (n->type == NODE_FLOAT_LIT) { printf("%g ", n->num_val); return; }
    if (n->type == NODE_IDENT)     { printf("%s ", n->str_val); return; }
    if (n->type == NODE_LOG)  { printf("log("); print_preorder(n->left); printf(") "); return; }
    if (n->type == NODE_EXP)  { printf("exp("); print_preorder(n->left); printf(") "); return; }
    printf("%s ", n->str_val);
    print_preorder(n->left);
    print_preorder(n->right);
}

static void print_inorder(ASTNode *n) {
    if (!n) return;
    if (n->type == NODE_FLOAT_LIT) { printf("%g", n->num_val); return; }
    if (n->type == NODE_IDENT)     { printf("%s", n->str_val); return; }
    if (n->type == NODE_LOG)  { printf("log("); print_inorder(n->left); printf(")"); return; }
    if (n->type == NODE_EXP)  { printf("exp("); print_inorder(n->left); printf(")"); return; }
    if (n->type == NODE_UNOP) { printf("(-"); print_inorder(n->left); printf(")"); return; }
    printf("(");
    print_inorder(n->left);
    printf(" %s ", n->str_val);
    print_inorder(n->right);
    printf(")");
}

static void print_postorder(ASTNode *n) {
    if (!n) return;
    if (n->type == NODE_FLOAT_LIT) { printf("%g ", n->num_val); return; }
    if (n->type == NODE_IDENT)     { printf("%s ", n->str_val); return; }
    if (n->type == NODE_LOG)  { print_postorder(n->left); printf("log "); return; }
    if (n->type == NODE_EXP)  { print_postorder(n->left); printf("exp "); return; }
    print_postorder(n->left);
    print_postorder(n->right);
    printf("%s ", n->str_val);
}

static double eval(ASTNode *n) {
    if (!n) return 0;
    switch (n->type) {
        case NODE_FLOAT_LIT: return n->num_val;
        case NODE_IDENT:     return 0; /* variables not bound in calculator mode */
        case NODE_LOG:       return log(eval(n->left));
        case NODE_EXP:       return exp(eval(n->left));
        case NODE_UNOP:      return -eval(n->left);
        case NODE_BINOP:
            switch (n->str_val[0]) {
                case '+': return eval(n->left) + eval(n->right);
                case '-': return eval(n->left) - eval(n->right);
                case '*': return eval(n->left) * eval(n->right);
                case '/': {
                    double d = eval(n->right);
                    if (d == 0) { fprintf(stderr, "Division by zero\n"); return 0; }
                    return eval(n->left) / d;
                }
                case '^': return pow(eval(n->left), eval(n->right));
            }
            break;
        default: break;
    }
    return 0;
}

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error at line %d: %s\n", yylineno, s);
}

int main(void) {
    printf("Infix Calculator with precedence, ^, log(), exp()\n");
    printf("Enter expressions (one per line), Ctrl+D to quit:\n\n");
    return yyparse();
}
