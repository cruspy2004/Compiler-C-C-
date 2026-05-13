/*
 * cse.c  —  Module 7: Common Subexpression Elimination.
 * Keeps a list of (arg1, op_rhs, dest_temp) seen so far.
 * For each new BINOP: if the same expression was already computed,
 * replace with ASSIGN from the existing temp.
 * Entries are invalidated when either operand is reassigned.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ir/tac.h"
#include "optimizer.h"

#define CSE_MAX 256

typedef struct {
    char *arg1;
    char *arg2;   /* full "op rhs" string */
    char *dest;
} CSEEntry;

static CSEEntry cse_table[CSE_MAX];
static int      cse_sz;

static void cse_clear(void) {
    for (int i = 0; i < cse_sz; i++) {
        free(cse_table[i].arg1); free(cse_table[i].arg2); free(cse_table[i].dest);
    }
    cse_sz = 0;
}

static const char *cse_lookup(const char *a1, const char *a2) {
    for (int i = 0; i < cse_sz; i++)
        if (cse_table[i].arg1 && cse_table[i].arg2 &&
            strcmp(cse_table[i].arg1, a1) == 0 &&
            strcmp(cse_table[i].arg2, a2) == 0)
            return cse_table[i].dest;
    return NULL;
}

static void cse_add(const char *a1, const char *a2, const char *dest) {
    if (cse_sz >= CSE_MAX) return;
    cse_table[cse_sz].arg1 = strdup(a1);
    cse_table[cse_sz].arg2 = strdup(a2);
    cse_table[cse_sz].dest = strdup(dest);
    cse_sz++;
}

/* Invalidate all entries that use `var` as arg1 or within arg2. */
static void cse_invalidate(const char *var) {
    if (!var) return;
    for (int i = 0; i < cse_sz; ) {
        int kill = 0;
        if (cse_table[i].arg1 && strcmp(cse_table[i].arg1, var) == 0) kill = 1;
        if (!kill && cse_table[i].arg2) {
            /* check if var appears after the operator in arg2 */
            char op_c; char rhs[64];
            if (sscanf(cse_table[i].arg2, " %c %63s", &op_c, rhs) == 2)
                if (strcmp(rhs, var) == 0) kill = 1;
        }
        if (kill) {
            free(cse_table[i].arg1); free(cse_table[i].arg2); free(cse_table[i].dest);
            cse_table[i] = cse_table[--cse_sz];
        } else {
            i++;
        }
    }
}

void pass_cse(TACList *list) {
    int elim = 0;
    cse_clear();
    for (TACInstr *ins = list->head; ins; ins = ins->next) {
        if (ins->op == TAC_BINOP && ins->arg1 && ins->arg2 && ins->result) {
            const char *existing = cse_lookup(ins->arg1, ins->arg2);
            if (existing) {
                printf("  [cse] %s = %s %s  =>  reuse %s\n",
                       ins->result, ins->arg1, ins->arg2, existing);
                /* Replace BINOP with ASSIGN from existing temp. */
                free(ins->arg1); free(ins->arg2);
                ins->op   = TAC_ASSIGN;
                ins->arg1 = strdup(existing);
                ins->arg2 = NULL;
                elim++;
            } else {
                cse_add(ins->arg1, ins->arg2, ins->result);
            }
        }
        /* Invalidate on assignment to result. */
        if (ins->result) cse_invalidate(ins->result);
    }
    if (elim) printf("  [cse] eliminated %d redundant computation(s)\n", elim);
    cse_clear();
}
