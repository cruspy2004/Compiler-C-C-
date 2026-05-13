#ifndef SCOPECHECKER_H
#define SCOPECHECKER_H

#include "../common/ast.h"
#include "symtab.h"

/*
 * Scope-checking pass.
 * Walks the AST and:
 *   - Pushes a new scope on BLOCK/FUNC_DEF entry.
 *   - Declares identifiers on DECL nodes.
 *   - Checks every IDENT use is declared.
 *   - Pops scope on block exit.
 *
 * Call before typecheck().
 */
void scopecheck(ASTNode *node, SymbolTable *st);

#endif /* SCOPECHECKER_H */
