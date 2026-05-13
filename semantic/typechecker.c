#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/ast.h"
#include "../common/error.h"
#include "symtab.h"
#include "typechecker.h"

/* Wrap a node in an implicit cast to float. */
static ASTNode *make_cast(ASTNode *node) {
    ASTNode *cast = ast_new_node(NODE_CAST, node->line);
    cast->data_type = TYPE_FLOAT;
    cast->left      = node;
    return cast;
}

DataType typecheck(ASTNode *node, SymbolTable *st) {
    if (!node) return TYPE_UNKNOWN;

    switch (node->type) {

        case NODE_INT_LIT:
            node->data_type = TYPE_INT;
            return TYPE_INT;

        case NODE_FLOAT_LIT:
            node->data_type = TYPE_FLOAT;
            return TYPE_FLOAT;

        case NODE_IDENT: {
            Symbol *sym = symtab_lookup(st, node->str_val);
            if (!sym) {
                /* scope checker should have caught this; still safe-guard */
                error_report(node->line, "undeclared variable '%s'", node->str_val);
                node->data_type = TYPE_UNKNOWN;
                return TYPE_UNKNOWN;
            }
            node->data_type = sym->type;
            return sym->type;
        }

        case NODE_UNOP: {
            DataType t = typecheck(node->left, st);
            node->data_type = (t == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
            return node->data_type;
        }

        case NODE_BINOP: {
            DataType lt = typecheck(node->left, st);
            DataType rt = typecheck(node->right, st);
            if (lt == TYPE_UNKNOWN || rt == TYPE_UNKNOWN) {
                node->data_type = TYPE_UNKNOWN;
                return TYPE_UNKNOWN;
            }
            if (lt == TYPE_FLOAT && rt == TYPE_INT) {
                node->right     = make_cast(node->right);
                node->data_type = TYPE_FLOAT;
            } else if (lt == TYPE_INT && rt == TYPE_FLOAT) {
                node->left      = make_cast(node->left);
                node->data_type = TYPE_FLOAT;
            } else if (lt == TYPE_FLOAT && rt == TYPE_FLOAT) {
                node->data_type = TYPE_FLOAT;
            } else {
                node->data_type = TYPE_INT;
            }
            return node->data_type;
        }

        case NODE_ASSIGN: {
            /* left is IDENT, right is expression */
            DataType ltype = typecheck(node->left, st);
            DataType rtype = typecheck(node->right, st);
            if (ltype == TYPE_INT && rtype == TYPE_FLOAT) {
                error_report(node->line,
                    "cannot assign float to int variable '%s'",
                    node->left->str_val ? node->left->str_val : "?");
            } else if (ltype == TYPE_FLOAT && rtype == TYPE_INT) {
                node->right = make_cast(node->right);
            }
            node->data_type = ltype;
            return ltype;
        }

        case NODE_DECL: {
            /* str_val holds type name, left is IDENT */
            DataType dt = (node->data_type == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
            if (node->left) node->left->data_type = dt;
            return dt;
        }

        case NODE_IF:
        case NODE_WHILE: {
            typecheck(node->extra, st); /* condition */
            typecheck(node->left,  st); /* then/body */
            typecheck(node->right, st); /* else */
            node->data_type = TYPE_UNKNOWN;
            return TYPE_UNKNOWN;
        }

        case NODE_RETURN: {
            DataType t = typecheck(node->left, st);
            node->data_type = t;
            return t;
        }

        case NODE_LOG:
        case NODE_EXP: {
            typecheck(node->left, st);
            node->data_type = TYPE_FLOAT;
            return TYPE_FLOAT;
        }

        case NODE_BLOCK:
        case NODE_STMT_LIST: {
            for (ASTNode *s = node->left; s; s = s->next)
                typecheck(s, st);
            node->data_type = TYPE_UNKNOWN;
            return TYPE_UNKNOWN;
        }

        case NODE_CALL: {
            /* check args */
            for (ASTNode *a = node->right; a; a = a->next)
                typecheck(a, st);
            node->data_type = TYPE_UNKNOWN;
            return TYPE_UNKNOWN;
        }

        default:
            return TYPE_UNKNOWN;
    }
}
