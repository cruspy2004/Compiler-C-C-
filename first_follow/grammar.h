#ifndef GRAMMAR_H
#define GRAMMAR_H

/*
 * grammar.h  —  shared grammar representation for Modules 4 programs.
 *
 * Convention:
 *   Uppercase letters  = non-terminals  (A-Z)
 *   Lowercase / punct  = terminals
 *   'e'                = epsilon (empty production)
 *   '$'                = end-of-input marker
 *
 * The target grammar from PRD FR-4 (LL(1) grammar):
 *   E  -> T E'
 *   E' -> + T E' | - T E' | e
 *   T  -> F T'
 *   T' -> * F T' | / F T' | e
 *   F  -> ( E ) | id | num
 */

#define MAX_SYMBOLS  30
#define MAX_PRODS    10
#define MAX_RHS_LEN  20
#define MAX_TERMINALS 30
#define EPSILON      'e'
#define ENDMARK      '$'

typedef struct {
    char lhs;
    char rhs[MAX_PRODS][MAX_RHS_LEN];
    int  num_prods;
} Production;

typedef struct {
    Production prods[MAX_SYMBOLS];
    int        num_prods;
    char       start;
    char       non_terminals[MAX_SYMBOLS];
    int        num_nts;
    char       terminals[MAX_TERMINALS];
    int        num_terms;
} Grammar;

/* Populate g with the target LL(1) grammar from the PRD. */
void grammar_load_default(Grammar *g);

/* Returns 1 if c is a non-terminal (uppercase). */
int is_nonterminal(char c);

/* Returns 1 if c is a terminal (not uppercase, not epsilon). */
int is_terminal(char c);

/* Find the Production entry for a given non-terminal lhs. Returns NULL if not found. */
Production *grammar_find_prod(Grammar *g, char lhs);

#endif /* GRAMMAR_H */
