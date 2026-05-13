#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "../ir/tac.h"

/* Bit-flags to select which passes to run. */
#define OPT_CONTROL_FLOW  (1 << 0)
#define OPT_CONST_FOLD    (1 << 1)
#define OPT_CONST_PROP    (1 << 2)
#define OPT_CSE           (1 << 3)
#define OPT_DEAD_CODE     (1 << 4)
#define OPT_LOOP          (1 << 5)
#define OPT_ALL           0x3F

/*
 * Run selected optimisation passes on the TAC list in-place.
 * Prints BEFORE/AFTER TAC for each enabled pass.
 */
void run_optimisations(TACList *list, int flags);

/* Individual pass entry points (called by run_optimisations). */
void pass_control_flow(TACList *list);
void pass_const_fold  (TACList *list);
void pass_const_prop  (TACList *list);
void pass_cse         (TACList *list);
void pass_dead_code   (TACList *list);
void pass_loop_opt    (TACList *list);

#endif /* OPTIMIZER_H */
