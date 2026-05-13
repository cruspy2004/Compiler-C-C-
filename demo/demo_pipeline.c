/*
 * demo_pipeline.c  —  Live demo of Modules 5-7.
 *
 * Manually builds the AST for this source program:
 *
 *   int x;
 *   int y;
 *   x = 3 + 4;          <- constant fold: 3+4 -> 7
 *   y = x * 2;
 *   int z;
 *   z = x * 2;          <- CSE: x*2 already computed -> reuse t1
 *   while (y != 0) {
 *       y = y - 1;      <- loop body
 *       z = x * 2;      <- LICM: x*2 is loop-invariant -> hoist
 *   }
 *   return z;
 *
 * Then runs:
 *   - Scope checker
 *   - Type checker
 *   - IR generator   -> prints unoptimised TAC
 *   - All 5 optimiser passes (with BEFORE/AFTER for each)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/ast.h"
#include "../common/error.h"
#include "../semantic/symtab.h"
#include "../semantic/scopechecker.h"
#include "../semantic/typechecker.h"
#include "../ir/tac.h"
#include "../ir/irgen.h"
#include "../optimizer/optimizer.h"

/* ---- AST construction helpers ---- */

static ASTNode *ident(const char *name) {
    ASTNode *n = ast_new_node(NODE_IDENT, 1);
    n->str_val = strdup(name);
    return n;
}

static ASTNode *int_lit(int v) {
    ASTNode *n = ast_new_node(NODE_INT_LIT, 1);
    n->num_val  = v;
    n->data_type = TYPE_INT;
    return n;
}

static ASTNode *binop(const char *op, ASTNode *l, ASTNode *r) {
    ASTNode *n = ast_new_node(NODE_BINOP, 1);
    n->str_val = strdup(op);
    n->left = l; n->right = r;
    return n;
}

static ASTNode *assign(const char *var, ASTNode *val) {
    ASTNode *n = ast_new_node(NODE_ASSIGN, 1);
    n->left  = ident(var);
    n->right = val;
    return n;
}

static ASTNode *decl(const char *type_name, const char *var_name) {
    ASTNode *n = ast_new_node(NODE_DECL, 1);
    n->str_val   = strdup(type_name);
    n->data_type = (strcmp(type_name, "float") == 0) ? TYPE_FLOAT : TYPE_INT;
    n->left      = ident(var_name);
    return n;
}

static ASTNode *ret(ASTNode *val) {
    ASTNode *n = ast_new_node(NODE_RETURN, 1);
    n->left = val;
    return n;
}

/* Chain a -> b -> c -> ... as next-sibling list */
static ASTNode *chain(ASTNode *a, ASTNode *b) {
    ASTNode *tail = a;
    while (tail->next) tail = tail->next;
    tail->next = b;
    return a;
}

int main(void) {
    printf("========================================================\n");
    printf(" CS-346 Mini-Compiler — Full Pipeline Demo\n");
    printf("========================================================\n");
    printf("\nSource program:\n");
    printf("  int x;\n");
    printf("  int y;\n");
    printf("  x = 3 + 4;       // constant fold -> 7\n");
    printf("  y = x * 2;\n");
    printf("  int z;\n");
    printf("  z = x * 2;       // CSE -> reuse computed x*2\n");
    printf("  while (y != 0) {\n");
    printf("      y = y - 1;\n");
    printf("      z = x * 2;   // LICM -> hoist out of loop\n");
    printf("  }\n");
    printf("  return z;\n\n");

    /* ---- Build AST manually ---- */

    /* while body: y = y - 1; z = x * 2; */
    ASTNode *body_y  = assign("y", binop("-", ident("y"), int_lit(1)));
    ASTNode *body_z2 = assign("z", binop("*", ident("x"), int_lit(2)));
    chain(body_y, body_z2);

    /* while (y != 0) { body } */
    ASTNode *while_cond = binop("!=", ident("y"), int_lit(0));
    ASTNode *while_node = ast_new_node(NODE_WHILE, 1);
    while_node->extra = while_cond;
    while_node->left  = body_y;  /* body head */

    /* Statement list */
    ASTNode *stmts =
        chain(decl("int", "x"),
        chain(decl("int", "y"),
        chain(assign("x", binop("+", int_lit(3), int_lit(4))),
        chain(assign("y", binop("*", ident("x"), int_lit(2))),
        chain(decl("int", "z"),
        chain(assign("z", binop("*", ident("x"), int_lit(2))),
        chain(while_node,
              ret(ident("z")))))))));

    /* ---- Phase 2: Scope checking ---- */
    printf("--------------------------------------------------------\n");
    printf(" PHASE 2 — Scope Checking\n");
    printf("--------------------------------------------------------\n");
    SymbolTable *st = symtab_create();
    symtab_push_scope(st);
    scopecheck(stmts, st);
    if (error_count() == 0)
        printf("  No scope errors.\n\n");
    else {
        error_summary();
        return 1;
    }

    /* ---- Phase 3: Type checking ---- */
    printf("--------------------------------------------------------\n");
    printf(" PHASE 3 — Type Checking\n");
    printf("--------------------------------------------------------\n");
    typecheck(stmts, st);
    if (error_count() == 0)
        printf("  No type errors.\n\n");
    else {
        error_summary();
        return 1;
    }

    /* ---- Phase 4: IR Generation ---- */
    printf("--------------------------------------------------------\n");
    printf(" PHASE 4 — Unoptimised Three-Address Code (TAC)\n");
    printf("--------------------------------------------------------\n");
    TACList *tac = irgen(stmts);
    tac_print(tac);
    printf("\n");

    /* ---- Phase 5: Optimisation ---- */
    printf("--------------------------------------------------------\n");
    printf(" PHASE 5 — Optimisation Passes\n");
    printf("--------------------------------------------------------\n");
    run_optimisations(tac, OPT_ALL);

    printf("\n--------------------------------------------------------\n");
    printf(" FINAL — Optimised TAC\n");
    printf("--------------------------------------------------------\n");
    tac_print(tac);

    tac_free(tac);
    symtab_destroy(st);
    return 0;
}
