#include <stdio.h>
#include <string.h>
#include "../common/ast.h"
#include "../common/error.h"
#include "symtab.h"
#include "scopechecker.h"

static DataType type_from_name(const char *name) {
    if (name && strcmp(name, "float") == 0) return TYPE_FLOAT;
    return TYPE_INT;
}

void scopecheck(ASTNode *node, SymbolTable *st) {
    if (!node) return;

    switch (node->type) {

        case NODE_DECL: {
            /* node->str_val = type name; node->left = IDENT node */
            DataType dt = type_from_name(node->str_val);
            node->data_type = dt;
            if (node->left && node->left->type == NODE_IDENT) {
                if (symtab_declare(st, node->left->str_val, dt) < 0) {
                    error_report(node->line,
                        "variable '%s' already declared in this scope",
                        node->left->str_val);
                }
            }
            /* Check initialiser expression if present. */
            if (node->right) scopecheck(node->right, st);
            break;
        }

        case NODE_IDENT: {
            Symbol *sym = symtab_lookup(st, node->str_val);
            if (!sym) {
                error_report(node->line,
                    "undeclared variable '%s'", node->str_val);
            }
            break;
        }

        case NODE_BLOCK:
        case NODE_FUNC_DEF: {
            symtab_push_scope(st);
            scopecheck(node->left,  st);
            scopecheck(node->right, st);
            scopecheck(node->extra, st);
            symtab_pop_scope(st);
            break;
        }

        case NODE_STMT_LIST: {
            for (ASTNode *s = node->left; s; s = s->next)
                scopecheck(s, st);
            break;
        }

        case NODE_IF: {
            /* extra = condition, left = then, right = else */
            scopecheck(node->extra, st);
            symtab_push_scope(st);
            scopecheck(node->left, st);
            symtab_pop_scope(st);
            if (node->right) {
                symtab_push_scope(st);
                scopecheck(node->right, st);
                symtab_pop_scope(st);
            }
            break;
        }

        case NODE_WHILE: {
            scopecheck(node->extra, st);
            symtab_push_scope(st);
            scopecheck(node->left, st);
            symtab_pop_scope(st);
            break;
        }

        case NODE_ASSIGN:
        case NODE_BINOP:
        case NODE_UNOP:
        case NODE_LOG:
        case NODE_EXP:
        case NODE_RETURN:
        case NODE_CAST: {
            scopecheck(node->left,  st);
            scopecheck(node->right, st);
            scopecheck(node->extra, st);
            break;
        }

        case NODE_CALL: {
            for (ASTNode *a = node->right; a; a = a->next)
                scopecheck(a, st);
            break;
        }

        /* Leaf nodes — nothing to check. */
        case NODE_INT_LIT:
        case NODE_FLOAT_LIT:
        case NODE_PARAM:
        case NODE_ARR_ACCESS:
        default:
            break;
    }

    /* Walk sibling statement list. */
    scopecheck(node->next, st);
}
