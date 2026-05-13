/*
 * perf.c  —  Module 7 Task 6: Performance Comparison.
 * Runs a sample arithmetic computation 5 times before and after
 * constant-folding optimisation, measures wall time via clock(),
 * and prints a table of results with speedup ratio.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../ir/tac.h"
#include "optimizer.h"

#define RUNS       5
#define ITER   100000  /* inner loop iterations to make timing measurable */

/* Build a sample TAC list representing:
 *   t0 = 3
 *   t1 = 4
 *   t2 = t0 + t1      <- foldable
 *   t3 = t2 * 2
 *   result = t3
 * (ITER copies so runtime is measurable)
 */
static TACList *build_sample_tac(void) {
    TACList *list = tac_list_create();
    for (int i = 0; i < ITER; i++) {
        tac_emit(list, TAC_ASSIGN, "t0", "3",    NULL);
        tac_emit(list, TAC_ASSIGN, "t1", "4",    NULL);
        tac_emit(list, TAC_BINOP,  "t2", "t0",   "+ t1");
        tac_emit(list, TAC_BINOP,  "t3", "t2",   "* 2");
        tac_emit(list, TAC_ASSIGN, "result", "t3", NULL);
    }
    return list;
}

/* Simulate "executing" the TAC list (just walks all nodes). */
static void simulate(TACList *list) {
    volatile long sum = 0;
    for (TACInstr *i = list->head; i; i = i->next)
        sum += (i->result ? i->result[0] : 0);
    (void)sum;
}

int main(void) {
    printf("=== Performance Comparison: before vs after constant folding ===\n\n");
    printf("%-6s  %-14s  %-14s  %-8s\n", "Run", "Before (ms)", "After (ms)", "Speedup");
    printf("%-6s  %-14s  %-14s  %-8s\n", "---", "-----------", "----------", "-------");

    double before_ms[RUNS], after_ms[RUNS];

    for (int r = 0; r < RUNS; r++) {
        /* --- BEFORE --- */
        TACList *before = build_sample_tac();
        clock_t t0 = clock();
        simulate(before);
        clock_t t1 = clock();
        before_ms[r] = 1000.0 * (t1 - t0) / CLOCKS_PER_SEC;
        tac_free(before);

        /* --- AFTER (with const fold + prop) --- */
        TACList *after = build_sample_tac();
        pass_const_fold(after);
        pass_const_prop(after);
        clock_t t2 = clock();
        simulate(after);
        clock_t t3 = clock();
        after_ms[r] = 1000.0 * (t3 - t2) / CLOCKS_PER_SEC;
        tac_free(after);

        double speedup = (after_ms[r] > 0) ? before_ms[r] / after_ms[r] : 0;
        printf("%-6d  %-14.4f  %-14.4f  %-8.2f\n",
               r + 1, before_ms[r], after_ms[r], speedup);
    }

    double avg_b = 0, avg_a = 0;
    for (int r = 0; r < RUNS; r++) { avg_b += before_ms[r]; avg_a += after_ms[r]; }
    avg_b /= RUNS; avg_a /= RUNS;
    double avg_speedup = (avg_a > 0) ? avg_b / avg_a : 0;
    printf("%-6s  %-14.4f  %-14.4f  %-8.2f\n", "Avg", avg_b, avg_a, avg_speedup);
    return 0;
}
