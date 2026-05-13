/*
 * dead_code.c  —  Module 7: Dead Code Elimination.
 * Build the set of variables ever used as an operand.
 * Walk backwards; remove any ASSIGN/BINOP/UNOP whose result is never used.
 * Repeat until stable (fixed-point).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ir/tac.h"
#include "optimizer.h"

#define USED_MAX 512

static char *used[USED_MAX];
static int   used_sz;

static void used_clear(void) {
    for (int i = 0; i < used_sz; i++) free(used[i]);
    used_sz = 0;
}

static int used_has(const char *v) {
    if (!v) return 0;
    for (int i = 0; i < used_sz; i++) if (strcmp(used[i], v) == 0) return 1;
    return 0;
}

static void used_add(const char *v) {
    if (!v || used_has(v)) return;
    if (used_sz < USED_MAX) used[used_sz++] = strdup(v);
}

/* Add all variable operands in an instruction to the used set. */
static void mark_uses(TACInstr *ins) {
    if (ins->op == TAC_LABEL) return;
    used_add(ins->arg1);
    /* For BINOP: arg2 = "<op> <rhs>"; extract rhs. */
    if (ins->op == TAC_BINOP && ins->arg2) {
        char op_c; char rhs[64];
        if (sscanf(ins->arg2, " %c %63s", &op_c, rhs) == 2)
            used_add(rhs);
    }
    if (ins->op == TAC_GOTO || ins->op == TAC_IF_GOTO) used_add(ins->result);
}

static int is_removable(TACInstr *ins) {
    return ins->op == TAC_ASSIGN || ins->op == TAC_BINOP || ins->op == TAC_UNOP;
}

void pass_dead_code(TACList *list) {
    int total = 0;
    int changed = 1;
    while (changed) {
        changed = 0;
        /* Build used set. */
        used_clear();
        for (TACInstr *ins = list->head; ins; ins = ins->next) mark_uses(ins);

        /* Remove instructions whose results are never used. */
        TACInstr *prev = NULL, *cur = list->head;
        while (cur) {
            if (is_removable(cur) && cur->result && !used_has(cur->result)) {
                printf("  [dead-code] removing dead assignment to '%s'\n", cur->result);
                TACInstr *dead = cur;
                cur = cur->next;
                if (prev) prev->next = cur;
                else       list->head = cur;
                if (dead == list->tail) list->tail = prev;
                free(dead->result); free(dead->arg1); free(dead->arg2); free(dead);
                total++; changed = 1;
            } else {
                prev = cur; cur = cur->next;
            }
        }
    }
    used_clear();
    if (total) printf("  [dead-code] removed %d dead instruction(s)\n", total);
}
