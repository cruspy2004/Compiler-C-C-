#ifndef TAC_H
#define TAC_H

typedef enum {
    TAC_ASSIGN,     /* result = arg1                    */
    TAC_BINOP,      /* result = arg1 op arg2            */
    TAC_UNOP,       /* result = op arg1                 */
    TAC_GOTO,       /* goto label                       */
    TAC_IF_GOTO,    /* if arg1 relop arg2 goto label    */
    TAC_PARAM,      /* param arg1                       */
    TAC_CALL,       /* result = call arg1, arg2(count)  */
    TAC_RETURN,     /* return arg1                      */
    TAC_ARR_LOAD,   /* result = arg1[arg2]              */
    TAC_ARR_STORE,  /* arg1[arg2] = result              */
    TAC_LABEL       /* label:                           */
} TACOpcode;

typedef struct TACInstr {
    TACOpcode        op;
    char            *result;   /* destination / label name */
    char            *arg1;     /* first operand / function name */
    char            *arg2;     /* second operand / arg count / relop */
    struct TACInstr *next;
} TACInstr;

typedef struct {
    TACInstr *head;
    TACInstr *tail;
    int       tmp_count;
    int       label_count;
} TACList;

/* Create an empty TAC list. */
TACList *tac_list_create(void);

/* Append an instruction (all string args are strdup'd). */
void tac_emit(TACList *list, TACOpcode op,
              const char *result, const char *arg1, const char *arg2);

/* Allocate a new temporary name "tN". Caller owns the returned string. */
char *tac_new_temp(TACList *list);

/* Allocate a new label name "LN". Caller owns the returned string. */
char *tac_new_label(TACList *list);

/* Print the entire TAC list to stdout. */
void tac_print(TACList *list);

/* Free all instructions and the list struct. */
void tac_free(TACList *list);

#endif /* TAC_H */
