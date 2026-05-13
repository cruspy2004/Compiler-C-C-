/*
 * first.c  —  Module 4: Compute FIRST sets for the LL(1) grammar.
 *
 * Algorithm: fixed-point iteration (repeat until no set changes).
 *   For each non-terminal X and production X -> a1 a2 ... ak:
 *     - If a1 is terminal:  add a1 to FIRST(X).
 *     - If a1 is NT:        add FIRST(a1) - {eps} to FIRST(X);
 *                           if eps in FIRST(a1), continue with a2, etc.
 *     - If all symbols can derive eps: add eps to FIRST(X).
 */

#include <stdio.h>
#include <string.h>
#include "grammar.h"

#define MAX_SET 40

typedef struct {
    char sym;
    char members[MAX_SET];
    int  size;
} FirstSet;

static FirstSet first_sets[MAX_SYMBOLS];
static int      num_sets;

static void set_add(FirstSet *s, char c) {
    for (int i = 0; i < s->size; i++)
        if (s->members[i] == c) return;
    s->members[s->size++] = c;
}

static int set_has(FirstSet *s, char c) {
    for (int i = 0; i < s->size; i++)
        if (s->members[i] == c) return 1;
    return 0;
}

static FirstSet *find_set(char sym) {
    for (int i = 0; i < num_sets; i++)
        if (first_sets[i].sym == sym) return &first_sets[i];
    return NULL;
}

static void compute_first(Grammar *g) {
    /* Initialise one set per non-terminal. */
    num_sets = g->num_nts;
    for (int i = 0; i < g->num_nts; i++) {
        first_sets[i].sym  = g->non_terminals[i];
        first_sets[i].size = 0;
    }

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int pi = 0; pi < g->num_prods; pi++) {
            Production *p = &g->prods[pi];
            FirstSet   *fs = find_set(p->lhs);
            if (!fs) continue;

            for (int ri = 0; ri < p->num_prods; ri++) {
                const char *rhs = p->rhs[ri];
                if (rhs[0] == EPSILON) {
                    /* X -> epsilon: add eps */
                    if (!set_has(fs, EPSILON)) { set_add(fs, EPSILON); changed = 1; }
                    continue;
                }
                int all_eps = 1;
                for (int si = 0; rhs[si] != '\0'; si++) {
                    char sym = rhs[si];
                    if (is_terminal(sym)) {
                        if (!set_has(fs, sym)) { set_add(fs, sym); changed = 1; }
                        all_eps = 0;
                        break;
                    }
                    /* sym is non-terminal */
                    FirstSet *sub = find_set(sym);
                    if (sub) {
                        for (int k = 0; k < sub->size; k++) {
                            if (sub->members[k] != EPSILON) {
                                if (!set_has(fs, sub->members[k])) {
                                    set_add(fs, sub->members[k]);
                                    changed = 1;
                                }
                            }
                        }
                        if (!set_has(sub, EPSILON)) { all_eps = 0; break; }
                    } else {
                        all_eps = 0; break;
                    }
                }
                if (all_eps) {
                    if (!set_has(fs, EPSILON)) { set_add(fs, EPSILON); changed = 1; }
                }
            }
        }
    }
}

static void print_first_sets(Grammar *g) {
    printf("=== FIRST Sets ===\n");
    for (int i = 0; i < g->num_nts; i++) {
        FirstSet *fs = find_set(g->non_terminals[i]);
        if (!fs) continue;
        /* Print NT name using original notation */
        char nt = fs->sym;
        if (nt == 'P') printf("FIRST(E') = { ");
        else if (nt == 'Q') printf("FIRST(T') = { ");
        else printf("FIRST(%c)  = { ", nt);
        for (int j = 0; j < fs->size; j++) {
            char c = fs->members[j];
            if (c == EPSILON) printf("epsilon");
            else if (c == ENDMARK) printf("$");
            else printf("%c", c);
            if (j < fs->size - 1) printf(", ");
        }
        printf(" }\n");
    }
}

int main(void) {
    Grammar g;
    grammar_load_default(&g);
    compute_first(&g);
    print_first_sets(&g);
    return 0;
}
