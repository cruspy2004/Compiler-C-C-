#include <stdio.h>
#include "../ir/tac.h"
#include "optimizer.h"

static void print_section(const char *title, TACList *list) {
    printf("\n=== %s ===\n", title);
    tac_print(list);
}

void run_optimisations(TACList *list, int flags) {
    if (flags & OPT_CONTROL_FLOW) {
        print_section("BEFORE control-flow (unreachable code)", list);
        pass_control_flow(list);
        print_section("AFTER  control-flow", list);
    }
    if (flags & OPT_CONST_FOLD) {
        print_section("BEFORE constant folding", list);
        pass_const_fold(list);
        print_section("AFTER  constant folding", list);
    }
    if (flags & OPT_CONST_PROP) {
        print_section("BEFORE constant propagation", list);
        pass_const_prop(list);
        print_section("AFTER  constant propagation", list);
    }
    if (flags & OPT_CSE) {
        print_section("BEFORE CSE", list);
        pass_cse(list);
        print_section("AFTER  CSE", list);
    }
    if (flags & OPT_DEAD_CODE) {
        print_section("BEFORE dead-code elimination", list);
        pass_dead_code(list);
        print_section("AFTER  dead-code elimination", list);
    }
    if (flags & OPT_LOOP) {
        print_section("BEFORE loop optimisation (LICM)", list);
        pass_loop_opt(list);
        print_section("AFTER  loop optimisation", list);
    }
}
