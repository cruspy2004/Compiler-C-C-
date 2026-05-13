/*
 * loop_opt.c  —  Module 7 Task 5: Loop-Invariant Code Motion (LICM).
 *
 * Detection heuristic:
 *   A loop is a pair (LABEL Lx, ..., GOTO Lx).
 *   An instruction in the loop body is loop-invariant if neither operand
 *   is a result defined inside the loop.
 *
 * Action: hoist the invariant instruction to just before the loop LABEL.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ir/tac.h"
#include "optimizer.h"

#define MAX_DEF 256

/* Collect all result names defined inside [start, end). */
static char *defs[MAX_DEF];
static int   ndefs;

static void collect_defs(TACInstr *start, TACInstr *end) {
    ndefs = 0;
    for (TACInstr *i = start; i && i != end; i = i->next) {
        if (i->result && ndefs < MAX_DEF) defs[ndefs++] = i->result;
    }
}

static int def_inside(const char *v) {
    if (!v) return 0;
    for (int i = 0; i < ndefs; i++) if (defs[i] && strcmp(defs[i], v) == 0) return 1;
    return 0;
}

static int is_invariant(TACInstr *ins) {
    if (ins->op != TAC_BINOP && ins->op != TAC_UNOP && ins->op != TAC_ASSIGN) return 0;
    if (def_inside(ins->arg1)) return 0;
    if (ins->op == TAC_BINOP && ins->arg2) {
        char op_c; char rhs[64];
        if (sscanf(ins->arg2, " %c %63s", &op_c, rhs) == 2)
            if (def_inside(rhs)) return 0;
    }
    return 1;
}

void pass_loop_opt(TACList *list) {
    int hoisted = 0;
    /* Find each LABEL, then find matching GOTO that targets it. */
    for (TACInstr *lbl = list->head; lbl; lbl = lbl->next) {
        if (lbl->op != TAC_LABEL || !lbl->result) continue;

        /* Find the GOTO that jumps back to this label. */
        TACInstr *back_goto = NULL;
        for (TACInstr *g = lbl->next; g; g = g->next) {
            if (g->op == TAC_GOTO && g->result && strcmp(g->result, lbl->result) == 0)
                { back_goto = g; break; }
        }
        if (!back_goto) continue;

        /* Collect definitions inside [lbl->next, back_goto]. */
        collect_defs(lbl->next, back_goto->next);

        /* Hoist invariant instructions. */
        TACInstr *prev = lbl;
        TACInstr *cur  = lbl->next;
        while (cur && cur != back_goto->next) {
            if (is_invariant(cur)) {
                printf("  [licm] hoisting: ");
                if (cur->op == TAC_ASSIGN)
                    printf("%s = %s", cur->result, cur->arg1);
                else
                    printf("%s = %s %s", cur->result, cur->arg1, cur->arg2 ? cur->arg2 : "");
                printf("\n");

                /* Unlink cur from its current position. */
                TACInstr *to_hoist = cur;
                prev->next = cur->next;
                cur = cur->next;
                if (to_hoist == list->tail) list->tail = prev;

                /* Insert to_hoist just before lbl. */
                /* Find node before lbl. */
                TACInstr *before_lbl = NULL;
                for (TACInstr *t = list->head; t && t->next != lbl; t = t->next)
                    before_lbl = t;
                if (before_lbl) {
                    to_hoist->next = lbl;
                    before_lbl->next = to_hoist;
                } else {
                    to_hoist->next = list->head;
                    list->head = to_hoist;
                }
                hoisted++;
            } else {
                prev = cur; cur = cur->next;
            }
        }
    }
    if (hoisted) printf("  [licm] hoisted %d instruction(s)\n", hoisted);
}
