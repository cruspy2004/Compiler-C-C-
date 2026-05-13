#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode *ast_new_node(NodeType type, int line) {
    ASTNode *n = (ASTNode *)calloc(1, sizeof(ASTNode));
    if (!n) { fprintf(stderr, "Out of memory\n"); exit(1); }
    n->type      = type;
    n->line      = line;
    n->data_type = TYPE_UNKNOWN;
    return n;
}

void ast_free(ASTNode *node) {
    if (!node) return;
    ast_free(node->left);
    ast_free(node->right);
    ast_free(node->extra);
    ast_free(node->next);
    free(node->str_val);
    free(node);
}

static const char *node_type_name(NodeType t) {
    switch (t) {
        case NODE_BINOP:      return "BINOP";
        case NODE_UNOP:       return "UNOP";
        case NODE_IDENT:      return "IDENT";
        case NODE_INT_LIT:    return "INT_LIT";
        case NODE_FLOAT_LIT:  return "FLOAT_LIT";
        case NODE_ASSIGN:     return "ASSIGN";
        case NODE_IF:         return "IF";
        case NODE_WHILE:      return "WHILE";
        case NODE_BLOCK:      return "BLOCK";
        case NODE_DECL:       return "DECL";
        case NODE_RETURN:     return "RETURN";
        case NODE_CALL:       return "CALL";
        case NODE_FUNC_DEF:   return "FUNC_DEF";
        case NODE_PARAM:      return "PARAM";
        case NODE_ARR_ACCESS: return "ARR_ACCESS";
        case NODE_CAST:       return "CAST";
        case NODE_LOG:        return "LOG";
        case NODE_EXP:        return "EXP";
        case NODE_STMT_LIST:  return "STMT_LIST";
        default:              return "UNKNOWN";
    }
}

void ast_print(ASTNode *node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) printf("  ");
    printf("[%s", node_type_name(node->type));
    if (node->str_val) printf(" '%s'", node->str_val);
    if (node->type == NODE_INT_LIT)   printf(" %d", (int)node->num_val);
    if (node->type == NODE_FLOAT_LIT) printf(" %g", node->num_val);
    printf("]\n");
    ast_print(node->left,  indent + 1);
    ast_print(node->right, indent + 1);
    ast_print(node->extra, indent + 1);
    ast_print(node->next,  indent);
}
