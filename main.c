/*
 * main.c  —  Pipeline entry point.
 *
 * Usage:  ./compiler <source-file>
 *
 * Phases executed in order:
 *   1. Lexer + Parser  (Flex/Bison) -> AST
 *   2. Scope checker                -> checks undeclared / redeclared vars
 *   3. Type checker                 -> annotates AST with types
 *   4. IR Generator                 -> emits TAC list
 *   5. Optimiser                    -> applies passes to TAC
 *   6. Code emission                -> writes a C file, invokes clang
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/ast.h"
#include "common/error.h"
#include "semantic/symtab.h"
#include "semantic/scopechecker.h"
#include "semantic/typechecker.h"
#include "ir/tac.h"
#include "ir/irgen.h"
#include "optimizer/optimizer.h"

/* Bison/Flex globals */
extern FILE    *yyin;
extern int      yyparse(void);
extern ASTNode *parse_root;   /* set by the Bison grammar action for the top rule */

/* ---- Emit optimised TAC as a simple C program ---- */
static void emit_c_from_tac(TACList *tac, const char *out_path) {
    FILE *f = fopen(out_path, "w");
    if (!f) { perror(out_path); return; }
    fprintf(f, "#include <stdio.h>\n#include <math.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    /* Generated from optimised TAC */\n");
    /* Simple: emit each TAC instruction as a comment for now. */
    for (TACInstr *ins = tac->head; ins; ins = ins->next) {
        switch (ins->op) {
            case TAC_LABEL:
                fprintf(f, "  %s: ;\n", ins->result);
                break;
            case TAC_ASSIGN:
                fprintf(f, "    double %s = %s;\n", ins->result, ins->arg1);
                break;
            case TAC_BINOP: {
                char op_c; char rhs[64];
                sscanf(ins->arg2, " %c %63s", &op_c, rhs);
                if (op_c == '^')
                    fprintf(f, "    double %s = pow(%s, %s);\n", ins->result, ins->arg1, rhs);
                else
                    fprintf(f, "    double %s = %s %c %s;\n", ins->result, ins->arg1, op_c, rhs);
                break;
            }
            case TAC_RETURN:
                fprintf(f, "    return %s;\n", ins->arg1 ? ins->arg1 : "0");
                break;
            default:
                break;
        }
    }
    fprintf(f, "    return 0;\n}\n");
    fclose(f);
    printf("[codegen] written to %s\n", out_path);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    /* --- Phase 1: Lexing + Parsing --- */
    FILE *src = fopen(argv[1], "r");
    if (!src) { perror(argv[1]); return 1; }
    yyin = src;
    printf("[pipeline] Parsing %s ...\n", argv[1]);
    int parse_err = yyparse();
    fclose(src);
    if (parse_err) {
        fprintf(stderr, "Parsing failed.\n");
        return 1;
    }

    /* parse_root is set by the parser. */
    extern ASTNode *parse_root;
    if (!parse_root) {
        fprintf(stderr, "Empty parse tree.\n");
        return 1;
    }

    printf("[pipeline] AST:\n");
    ast_print(parse_root, 0);

    /* --- Phase 2: Scope checking --- */
    printf("\n[pipeline] Scope checking ...\n");
    SymbolTable *st = symtab_create();
    symtab_push_scope(st);
    scopecheck(parse_root, st);
    if (error_count() > 0) {
        error_summary();
        return 1;
    }

    /* --- Phase 3: Type checking --- */
    printf("[pipeline] Type checking ...\n");
    typecheck(parse_root, st);
    if (error_count() > 0) {
        error_summary();
        return 1;
    }

    /* --- Phase 4: IR Generation --- */
    printf("\n[pipeline] Generating TAC IR ...\n");
    TACList *tac = irgen(parse_root);
    printf("--- Unoptimised TAC ---\n");
    tac_print(tac);

    /* --- Phase 5: Optimisation --- */
    printf("\n[pipeline] Running optimisations ...\n");
    run_optimisations(tac, OPT_ALL);
    printf("\n--- Optimised TAC ---\n");
    tac_print(tac);

    /* --- Phase 6: Code emission --- */
    char out_c[256];
    snprintf(out_c, sizeof(out_c), "%s.out.c", argv[1]);
    emit_c_from_tac(tac, out_c);

    /* Invoke clang to compile to a native binary. */
    char cmd[512];
    char out_bin[256];
    snprintf(out_bin, sizeof(out_bin), "%s.out", argv[1]);
    snprintf(cmd, sizeof(cmd), "clang %s -o %s -lm 2>&1", out_c, out_bin);
    printf("[codegen] running: %s\n", cmd);
    int ret = system(cmd);
    if (ret == 0) printf("[codegen] binary: %s\n", out_bin);
    else          printf("[codegen] clang not available or compile error (exit %d)\n", ret);

    tac_free(tac);
    symtab_destroy(st);
    ast_free(parse_root);
    return 0;
}
