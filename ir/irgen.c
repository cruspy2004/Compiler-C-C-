/*
 * irgen.c  —  Module 6: Three-Address Code generator.
 * Walks the annotated AST and emits TAC instructions.
 *
 * TAC instruction format:
 *   BINOP : result = arg1 <op> arg2   (arg2 holds operator symbol in result
 *                                      field, see tac_emit usage below)
 *
 * We store BINOP as:  result=dest, arg1=lhs, arg2="op rhs"  for easy printing.
 * The optimiser treats arg2 as the operator and looks up arg2 separately where
 * needed.  A cleaner representation stores op in a dedicated field, but the
 * struct defined in tac.h is kept simple.
 *
 * Actual storage used here:
 *   TAC_BINOP:  result=dest  arg1=lhs  arg2=<"op rhs"> concatenated string.
 * The tac_print function in tac.c is updated to split on the first space.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/ast.h"
#include "tac.h"
#include "irgen.h"

/* Forward declaration. */
static char *gen_expr(TACList *list, ASTNode *node);
static void  gen_stmt(TACList *list, ASTNode *node);

/* ---- helpers ---- */

static char *num_to_str(double v, DataType dt) {
    char *buf = malloc(32);
    if (dt == TYPE_FLOAT)
        snprintf(buf, 32, "%g", v);
    else
        snprintf(buf, 32, "%d", (int)v);
    return buf;
}

/* Emit a BINOP instruction and return the destination temp. */
static char *emit_binop(TACList *list, const char *lhs, const char *op, const char *rhs) {
    char *dest = tac_new_temp(list);
    /* Pack "op rhs" into arg2 for printing convenience. */
    char arg2[128];
    snprintf(arg2, sizeof(arg2), "%s %s", op, rhs);
    tac_emit(list, TAC_BINOP, dest, lhs, arg2);
    return dest;
}

/* ---- expression codegen ---- */

/* Returns the name (temp or identifier) that holds the expression value. */
static char *gen_expr(TACList *list, ASTNode *node) {
    if (!node) return strdup("?");

    switch (node->type) {

        case NODE_INT_LIT:
        case NODE_FLOAT_LIT: {
            char *lit = num_to_str(node->num_val, node->data_type);
            char *dest = tac_new_temp(list);
            tac_emit(list, TAC_ASSIGN, dest, lit, NULL);
            free(lit);
            return dest;
        }

        case NODE_IDENT:
            return strdup(node->str_val);

        case NODE_CAST: {
            char *inner = gen_expr(list, node->left);
            char *dest  = tac_new_temp(list);
            tac_emit(list, TAC_UNOP, dest, inner, NULL);  /* (float) cast */
            free(inner);
            return dest;
        }

        case NODE_UNOP: {
            char *inner = gen_expr(list, node->left);
            char *dest  = tac_new_temp(list);
            tac_emit(list, TAC_UNOP, dest, inner, NULL);
            free(inner);
            return dest;
        }

        case NODE_BINOP: {
            char *lhs = gen_expr(list, node->left);
            char *rhs = gen_expr(list, node->right);
            char *dest = emit_binop(list, lhs, node->str_val, rhs);
            free(lhs); free(rhs);
            return dest;
        }

        case NODE_LOG: {
            char *inner = gen_expr(list, node->left);
            char *dest  = tac_new_temp(list);
            /* Represent as a call to log */
            tac_emit(list, TAC_PARAM, NULL, inner, NULL);
            tac_emit(list, TAC_CALL, dest, "log", "1");
            free(inner);
            return dest;
        }

        case NODE_EXP: {
            char *inner = gen_expr(list, node->left);
            char *dest  = tac_new_temp(list);
            tac_emit(list, TAC_PARAM, NULL, inner, NULL);
            tac_emit(list, TAC_CALL, dest, "exp", "1");
            free(inner);
            return dest;
        }

        case NODE_CALL: {
            int argc = 0;
            for (ASTNode *a = node->right; a; a = a->next) {
                char *av = gen_expr(list, a);
                tac_emit(list, TAC_PARAM, NULL, av, NULL);
                free(av);
                argc++;
            }
            char argc_str[16];
            snprintf(argc_str, sizeof(argc_str), "%d", argc);
            char *dest = tac_new_temp(list);
            tac_emit(list, TAC_CALL, dest, node->str_val, argc_str);
            return dest;
        }

        case NODE_ARR_ACCESS: {
            char *arr = strdup(node->str_val);
            char *idx = gen_expr(list, node->left);
            char *dest = tac_new_temp(list);
            tac_emit(list, TAC_ARR_LOAD, dest, arr, idx);
            free(arr); free(idx);
            return dest;
        }

        default:
            return strdup("?");
    }
}

/* ---- statement codegen ---- */

static void gen_stmt(TACList *list, ASTNode *node) {
    if (!node) return;

    switch (node->type) {

        case NODE_ASSIGN: {
            char *rhs = gen_expr(list, node->right);
            if (node->left->type == NODE_ARR_ACCESS) {
                char *idx = gen_expr(list, node->left->left);
                tac_emit(list, TAC_ARR_STORE, rhs, node->left->str_val, idx);
                free(idx);
            } else {
                tac_emit(list, TAC_ASSIGN, node->left->str_val, rhs, NULL);
            }
            free(rhs);
            break;
        }

        case NODE_DECL:
            /* Variable declaration — no TAC needed unless there's an initialiser. */
            if (node->right) {
                char *rhs = gen_expr(list, node->right);
                if (node->left)
                    tac_emit(list, TAC_ASSIGN, node->left->str_val, rhs, NULL);
                free(rhs);
            }
            break;

        case NODE_IF: {
            char *cond_label = tac_new_label(list);  /* false branch */
            char *end_label  = tac_new_label(list);  /* past else */

            /* Evaluate condition */
            char *cond = gen_expr(list, node->extra);
            /* if cond == 0 goto cond_label */
            tac_emit(list, TAC_IF_GOTO, cond_label, cond, "== 0");
            free(cond);

            /* then branch */
            gen_stmt(list, node->left);
            tac_emit(list, TAC_GOTO, end_label, NULL, NULL);

            /* else branch */
            tac_emit(list, TAC_LABEL, cond_label, NULL, NULL);
            if (node->right) gen_stmt(list, node->right);

            tac_emit(list, TAC_LABEL, end_label, NULL, NULL);
            free(cond_label); free(end_label);
            break;
        }

        case NODE_WHILE: {
            char *loop_label = tac_new_label(list);
            char *exit_label = tac_new_label(list);

            tac_emit(list, TAC_LABEL, loop_label, NULL, NULL);
            char *cond = gen_expr(list, node->extra);
            tac_emit(list, TAC_IF_GOTO, exit_label, cond, "== 0");
            free(cond);

            gen_stmt(list, node->left);  /* body */
            tac_emit(list, TAC_GOTO, loop_label, NULL, NULL);
            tac_emit(list, TAC_LABEL, exit_label, NULL, NULL);
            free(loop_label); free(exit_label);
            break;
        }

        case NODE_RETURN: {
            if (node->left) {
                char *val = gen_expr(list, node->left);
                tac_emit(list, TAC_RETURN, NULL, val, NULL);
                free(val);
            } else {
                tac_emit(list, TAC_RETURN, NULL, NULL, NULL);
            }
            break;
        }

        case NODE_BLOCK:
        case NODE_STMT_LIST: {
            for (ASTNode *s = node->left; s; s = s->next)
                gen_stmt(list, s);
            break;
        }

        case NODE_FUNC_DEF: {
            /* Emit a label for the function entry. */
            tac_emit(list, TAC_LABEL, node->str_val, NULL, NULL);
            gen_stmt(list, node->left);  /* body */
            break;
        }

        default:
            /* Expression used as statement (e.g., function call). */
            {
                char *v = gen_expr(list, node);
                free(v);
            }
            break;
    }

    /* Walk sibling list. */
    gen_stmt(list, node->next);
}

/* ---- public API ---- */

TACList *irgen(ASTNode *root) {
    TACList *list = tac_list_create();
    gen_stmt(list, root);
    return list;
}
