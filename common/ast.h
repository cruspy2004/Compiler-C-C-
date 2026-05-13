#ifndef AST_H
#define AST_H

/* Forward declaration for DataType used by both AST and semantic phases. */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_UNKNOWN
} DataType;

/* Node type tags for the AST. */
typedef enum {
    NODE_BINOP,      /* binary operator: +  -  *  /  ^ */
    NODE_UNOP,       /* unary operator:  - */
    NODE_IDENT,      /* identifier */
    NODE_INT_LIT,    /* integer literal */
    NODE_FLOAT_LIT,  /* float literal */
    NODE_ASSIGN,     /* x = expr */
    NODE_IF,         /* if (cond) then else */
    NODE_WHILE,      /* while (cond) body */
    NODE_BLOCK,      /* { stmt; stmt; ... } */
    NODE_DECL,       /* type id; */
    NODE_RETURN,     /* return expr */
    NODE_CALL,       /* function call f(args) */
    NODE_FUNC_DEF,   /* function definition */
    NODE_PARAM,      /* parameter in function def */
    NODE_ARR_ACCESS, /* a[i] */
    NODE_CAST,       /* implicit int->float cast */
    NODE_LOG,        /* log(expr) */
    NODE_EXP,        /* exp(expr) */
    NODE_STMT_LIST   /* linked list of statements */
} NodeType;

typedef struct ASTNode {
    NodeType     type;
    char        *str_val;        /* identifier name, operator string */
    double       num_val;        /* numeric literal value */
    DataType     data_type;      /* filled in by type checker */
    int          line;           /* source line number */
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *extra;       /* else branch, arg list, condition */
    struct ASTNode *next;        /* sibling in statement list */
} ASTNode;

/* Allocate and zero-initialise a new AST node. */
ASTNode *ast_new_node(NodeType type, int line);

/* Recursively free an AST subtree. */
void ast_free(ASTNode *node);

/* Print the AST in S-expression form for debugging. */
void ast_print(ASTNode *node, int indent);

#endif /* AST_H */
