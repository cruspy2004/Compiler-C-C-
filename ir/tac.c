#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tac.h"

TACList *tac_list_create(void) {
    return (TACList *)calloc(1, sizeof(TACList));
}

static char *dup(const char *s) { return s ? strdup(s) : NULL; }

void tac_emit(TACList *list, TACOpcode op,
              const char *result, const char *arg1, const char *arg2) {
    TACInstr *ins = (TACInstr *)calloc(1, sizeof(TACInstr));
    ins->op     = op;
    ins->result = dup(result);
    ins->arg1   = dup(arg1);
    ins->arg2   = dup(arg2);
    if (list->tail) list->tail->next = ins;
    else            list->head = ins;
    list->tail = ins;
}

char *tac_new_temp(TACList *list) {
    char buf[32];
    snprintf(buf, sizeof(buf), "t%d", list->tmp_count++);
    return strdup(buf);
}

char *tac_new_label(TACList *list) {
    char buf[32];
    snprintf(buf, sizeof(buf), "L%d", list->label_count++);
    return strdup(buf);
}

static const char *opcode_str(TACOpcode op) {
    switch (op) {
        case TAC_ASSIGN:    return "ASSIGN";
        case TAC_BINOP:     return "BINOP";
        case TAC_UNOP:      return "UNOP";
        case TAC_GOTO:      return "GOTO";
        case TAC_IF_GOTO:   return "IF_GOTO";
        case TAC_PARAM:     return "PARAM";
        case TAC_CALL:      return "CALL";
        case TAC_RETURN:    return "RETURN";
        case TAC_ARR_LOAD:  return "ARR_LOAD";
        case TAC_ARR_STORE: return "ARR_STORE";
        case TAC_LABEL:     return "LABEL";
        default:            return "?";
    }
}

void tac_print(TACList *list) {
    for (TACInstr *i = list->head; i; i = i->next) {
        switch (i->op) {
            case TAC_LABEL:
                printf("%s:\n", i->result);
                break;
            case TAC_ASSIGN:
                printf("    %s = %s\n", i->result, i->arg1);
                break;
            case TAC_BINOP:
                /* arg2 = "op rhs", e.g. "+ t1" */
                printf("    %s = %s %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_UNOP:
                printf("    %s = -%s\n", i->result, i->arg1);
                break;
            case TAC_GOTO:
                printf("    goto %s\n", i->result);
                break;
            case TAC_IF_GOTO:
                /* arg1=lhs, arg2="relop rhs", result=label */
                printf("    if %s %s goto %s\n", i->arg1, i->arg2, i->result);
                break;
            case TAC_PARAM:
                printf("    param %s\n", i->arg1);
                break;
            case TAC_CALL:
                printf("    %s = call %s, %s\n",
                       i->result ? i->result : "_",
                       i->arg1, i->arg2);
                break;
            case TAC_RETURN:
                printf("    return %s\n", i->arg1 ? i->arg1 : "");
                break;
            case TAC_ARR_LOAD:
                printf("    %s = %s[%s]\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_ARR_STORE:
                printf("    %s[%s] = %s\n", i->arg1, i->arg2, i->result);
                break;
            default:
                printf("    [%s]\n", opcode_str(i->op));
        }
    }
}

void tac_free(TACList *list) {
    TACInstr *cur = list->head;
    while (cur) {
        TACInstr *nxt = cur->next;
        free(cur->result);
        free(cur->arg1);
        free(cur->arg2);
        free(cur);
        cur = nxt;
    }
    free(list);
}
