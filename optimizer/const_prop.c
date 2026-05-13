/*
 * const_prop.c  —  Module 7: Constant Propagation.
 * Maintains a map var -> constant.
 * For each TAC_ASSIGN "x = literal": record x=literal.
 * For subsequent instructions: replace uses of x with literal.
 * Invalidate when x is reassigned.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../ir/tac.h"
#include "optimizer.h"

#define MAP_SIZE 256

typedef struct { char *var; char *val; } MapEntry;
static MapEntry cmap[MAP_SIZE];
static int      cmap_sz;

static void map_clear(void) {
    for (int i = 0; i < cmap_sz; i++) { free(cmap[i].var); free(cmap[i].val); }
    cmap_sz = 0;
}

static const char *map_get(const char *var) {
    if (!var) return NULL;
    for (int i = 0; i < cmap_sz; i++)
        if (strcmp(cmap[i].var, var) == 0) return cmap[i].val;
    return NULL;
}

static void map_set(const char *var, const char *val) {
    if (!var) return;
    for (int i = 0; i < cmap_sz; i++) {
        if (strcmp(cmap[i].var, var) == 0) {
            free(cmap[i].val); cmap[i].val = strdup(val); return;
        }
    }
    if (cmap_sz < MAP_SIZE) {
        cmap[cmap_sz].var = strdup(var);
        cmap[cmap_sz].val = strdup(val);
        cmap_sz++;
    }
}

static void map_invalidate(const char *var) {
    if (!var) return;
    for (int i = 0; i < cmap_sz; i++) {
        if (strcmp(cmap[i].var, var) == 0) {
            free(cmap[i].var); free(cmap[i].val);
            cmap[i] = cmap[--cmap_sz];
            return;
        }
    }
}

static int is_numeric(const char *s) {
    if (!s || !*s) return 0;
    const char *p = s; if (*p=='-') p++;
    int hd=0; while(isdigit((unsigned char)*p)){hd=1;p++;}
    if(*p=='.'){p++;while(isdigit((unsigned char)*p))p++;}
    return hd && (*p=='\0');
}

static char *replace_if_known(const char *s) {
    if (!s) return NULL;
    const char *v = map_get(s);
    return v ? strdup(v) : strdup(s);
}

void pass_const_prop(TACList *list) {
    int subs = 0;
    map_clear();
    for (TACInstr *ins = list->head; ins; ins = ins->next) {
        /* Propagate into uses first. */
        if (ins->op == TAC_ASSIGN || ins->op == TAC_BINOP || ins->op == TAC_UNOP) {
            char *na1 = replace_if_known(ins->arg1);
            if (ins->arg1 && strcmp(na1, ins->arg1) != 0) {
                printf("  [const-prop] replaced %s -> %s in instr\n", ins->arg1, na1);
                free(ins->arg1); ins->arg1 = na1; subs++;
            } else { free(na1); }
        }
        if (ins->op == TAC_BINOP && ins->arg2) {
            /* arg2 = "<op> <rhs>", substitute rhs */
            char op_c; char rhs[64];
            if (sscanf(ins->arg2, " %c %63s", &op_c, rhs) == 2) {
                const char *v = map_get(rhs);
                if (v) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "%c %s", op_c, v);
                    printf("  [const-prop] replaced %s -> %s in BINOP arg\n", rhs, v);
                    free(ins->arg2); ins->arg2 = strdup(buf); subs++;
                }
            }
        }
        /* Record new constants and invalidate overwritten vars. */
        if (ins->op == TAC_ASSIGN && is_numeric(ins->arg1)) {
            map_set(ins->result, ins->arg1);
        } else if (ins->result) {
            map_invalidate(ins->result);
        }
    }
    if (subs) printf("  [const-prop] %d substitution(s)\n", subs);
    map_clear();
}
