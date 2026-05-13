#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "../common/ast.h"
#include "symtab.h"

/*
 * Type-checking pass.
 * Walks the annotated AST (after scope checking) and:
 *   - Assigns data_type to every expression node.
 *   - Inserts NODE_CAST nodes for implicit int->float promotions.
 *   - Reports type errors via error_report().
 *
 * Returns the DataType of the root node.
 */
DataType typecheck(ASTNode *node, SymbolTable *st);

#endif /* TYPECHECKER_H */
