/*
 * const_fold.c  —  Module 7: Constant Folding.
 * For TAC_BINOP where both arg1 and the rhs part of arg2 are numeric literals,
 * evaluate at compile time and replace with TAC_ASSIGN of the result.
 *
 * TAC BINOP format: result = arg1  arg2
 *                   where arg2 = "<op> <rhs_operand>"
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "../ir/tac.h"
#include "optimizer.h"

static int is_numeric(const char *s) {
    if (!s || !*s) return 0;
    const char *p = s;
    if (*p == '-') p++;
    int has_digit = 0;
    while (isdigit((unsigned char)*p)) { has_digit = 1; p++; }
    if (*p == '.') { p++; while (isdigit((unsigned char)*p)) p++; }
    return has_digit && (*p == '\0');
}

void pass_const_fold(TACList *list) {
    int folded = 0;
    for (TACInstr *ins = list->head; ins; ins = ins->next) {
        if (ins->op != TAC_BINOP) continue;
        /* arg2 format: "<op> <rhs>" */
        if (!ins->arg1 || !ins->arg2) continue;
        char op_char;
        char rhs_str[64];
        if (sscanf(ins->arg2, " %c %63s", &op_char, rhs_str) != 2) continue;
        if (!is_numeric(ins->arg1) || !is_numeric(rhs_str)) continue;

        double a = atof(ins->arg1);
        double b = atof(rhs_str);
        double result;
        switch (op_char) {
            case '+': result = a + b; break;
            case '-': result = a - b; break;
            case '*': result = a * b; break;
            case '/': if (b == 0) continue; result = a / b; break;
            case '^': result = pow(a, b); break;
            default:  continue;
        }
        /* Replace BINOP with ASSIGN of the folded constant. */
        char buf[32];
        /* Preserve int if both operands and result are integral. */
        if (result == (long long)result && op_char != '/')
            snprintf(buf, sizeof(buf), "%lld", (long long)result);
        else
            snprintf(buf, sizeof(buf), "%g", result);

        ins->op = TAC_ASSIGN;
        free(ins->arg1); ins->arg1 = strdup(buf);
        free(ins->arg2); ins->arg2 = NULL;
        folded++;
        printf("  [const-fold] %s = %s %s  =>  %s = %s\n",
               ins->result, ins->arg1, "folded", ins->result, buf);
    }
    if (folded)
        printf("  [const-fold] folded %d instruction(s)\n", folded);
}
