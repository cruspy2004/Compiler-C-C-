#ifndef SYMTAB_H
#define SYMTAB_H

#include "../common/ast.h"

#define SYMTAB_BUCKETS 64

typedef struct Symbol {
    char             name[64];
    DataType         type;
    int              scope_level;
    struct Symbol   *next;       /* hash chain */
} Symbol;

typedef struct ScopeFrame {
    Symbol          *buckets[SYMTAB_BUCKETS];
    struct ScopeFrame *prev;
    int              level;
} ScopeFrame;

typedef struct {
    ScopeFrame *top;
    int         level;
} SymbolTable;

/* Create and destroy the symbol table. */
SymbolTable *symtab_create(void);
void         symtab_destroy(SymbolTable *t);

/* Scope management. */
void symtab_push_scope(SymbolTable *t);
void symtab_pop_scope (SymbolTable *t);

/* Declare a new symbol in the current scope.
   Returns 0 on success, -1 if already declared in this scope. */
int symtab_declare(SymbolTable *t, const char *name, DataType type);

/* Look up a symbol from innermost scope outward. Returns NULL if not found. */
Symbol *symtab_lookup(SymbolTable *t, const char *name);

/* Look up only in the current (innermost) scope. */
Symbol *symtab_lookup_current(SymbolTable *t, const char *name);

/* Print the entire symbol table (for debugging). */
void symtab_dump(SymbolTable *t);

#endif /* SYMTAB_H */
