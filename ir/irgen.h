#ifndef IRGEN_H
#define IRGEN_H

#include "../common/ast.h"
#include "tac.h"

/*
 * irgen() walks the annotated AST and emits Three-Address Code.
 * Returns a TACList owned by the caller (free with tac_free()).
 */
TACList *irgen(ASTNode *root);

#endif /* IRGEN_H */
