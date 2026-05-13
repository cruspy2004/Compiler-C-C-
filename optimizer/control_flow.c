/*
 * control_flow.c  —  Module 7 Task 4
 * Remove unreachable instructions after unconditional TAC_GOTO.
 * Any instruction between a GOTO and the next LABEL is dead.
 */
#include <stdio.h>
#include <stdlib.h>
#include "../ir/tac.h"
#include "optimizer.h"

void pass_control_flow(TACList *list) {
    int removed = 0;
    TACInstr *prev = NULL;
    TACInstr *cur  = list->head;
    int unreachable = 0;

    while (cur) {
        if (unreachable && cur->op != TAC_LABEL) {
            /* Remove this unreachable instruction. */
            TACInstr *dead = cur;
            cur = cur->next;
            if (prev) prev->next = cur;
            else       list->head = cur;
            if (dead == list->tail) list->tail = prev;
            free(dead->result); free(dead->arg1); free(dead->arg2);
            free(dead);
            removed++;
            continue;
        }
        if (cur->op == TAC_LABEL)  unreachable = 0;
        if (cur->op == TAC_GOTO)   unreachable = 1;
        prev = cur;
        cur  = cur->next;
    }
    if (removed)
        printf("  [control-flow] removed %d unreachable instruction(s)\n", removed);
}
