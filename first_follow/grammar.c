#include <string.h>
#include <ctype.h>
#include "grammar.h"

/*
 * Default grammar (from PRD FR-4):
 *   E  -> T E'       (non-terminal E-prime represented as 'P')
 *   P  -> + T P | - T P | e
 *   T  -> F T'       (non-terminal T-prime represented as 'Q')
 *   Q  -> * F Q | / F Q | e
 *   F  -> ( E ) | id | num
 *
 * We use single chars: 'P' = E', 'Q' = T'
 * Terminals: +  -  *  /  (  )  i(d)  n(um)
 */
void grammar_load_default(Grammar *g) {
    memset(g, 0, sizeof(*g));
    g->start = 'E';

    /* E -> T P */
    g->prods[0].lhs = 'E';
    strcpy(g->prods[0].rhs[0], "TP");
    g->prods[0].num_prods = 1;

    /* P -> +TP | -TP | e */
    g->prods[1].lhs = 'P';
    strcpy(g->prods[1].rhs[0], "+TP");
    strcpy(g->prods[1].rhs[1], "-TP");
    strcpy(g->prods[1].rhs[2], "e");
    g->prods[1].num_prods = 3;

    /* T -> F Q */
    g->prods[2].lhs = 'T';
    strcpy(g->prods[2].rhs[0], "FQ");
    g->prods[2].num_prods = 1;

    /* Q -> *FQ | /FQ | e */
    g->prods[3].lhs = 'Q';
    strcpy(g->prods[3].rhs[0], "*FQ");
    strcpy(g->prods[3].rhs[1], "/FQ");
    strcpy(g->prods[3].rhs[2], "e");
    g->prods[3].num_prods = 3;

    /* F -> (E) | i | n */
    g->prods[4].lhs = 'F';
    strcpy(g->prods[4].rhs[0], "(E)");
    strcpy(g->prods[4].rhs[1], "i");
    strcpy(g->prods[4].rhs[2], "n");
    g->prods[4].num_prods = 3;

    g->num_prods = 5;

    /* non-terminals */
    g->non_terminals[0] = 'E';
    g->non_terminals[1] = 'P';
    g->non_terminals[2] = 'T';
    g->non_terminals[3] = 'Q';
    g->non_terminals[4] = 'F';
    g->num_nts = 5;

    /* terminals */
    g->terminals[0] = '+';
    g->terminals[1] = '-';
    g->terminals[2] = '*';
    g->terminals[3] = '/';
    g->terminals[4] = '(';
    g->terminals[5] = ')';
    g->terminals[6] = 'i';
    g->terminals[7] = 'n';
    g->terminals[8] = ENDMARK;
    g->num_terms = 9;
}

int is_nonterminal(char c) { return (c >= 'A' && c <= 'Z'); }

int is_terminal(char c) {
    return !is_nonterminal(c) && c != EPSILON && c != '\0';
}

Production *grammar_find_prod(Grammar *g, char lhs) {
    for (int i = 0; i < g->num_prods; i++)
        if (g->prods[i].lhs == lhs)
            return &g->prods[i];
    return NULL;
}
