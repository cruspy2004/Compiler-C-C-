#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* djb2 hash for a string. */
static unsigned int hash_str(const char *s) {
    unsigned int h = 5381;
    while (*s) h = ((h << 5) + h) + (unsigned char)(*s++);
    return h % SYMTAB_BUCKETS;
}

SymbolTable *symtab_create(void) {
    SymbolTable *t = calloc(1, sizeof(SymbolTable));
    return t;
}

void symtab_destroy(SymbolTable *t) {
    while (t->top) symtab_pop_scope(t);
    free(t);
}

void symtab_push_scope(SymbolTable *t) {
    ScopeFrame *f = calloc(1, sizeof(ScopeFrame));
    f->prev  = t->top;
    f->level = ++t->level;
    t->top   = f;
}

void symtab_pop_scope(SymbolTable *t) {
    if (!t->top) return;
    ScopeFrame *f = t->top;
    t->top = f->prev;
    t->level--;
    /* Free all symbols in this frame. */
    for (int i = 0; i < SYMTAB_BUCKETS; i++) {
        Symbol *s = f->buckets[i];
        while (s) {
            Symbol *nxt = s->next;
            free(s);
            s = nxt;
        }
    }
    free(f);
}

int symtab_declare(SymbolTable *t, const char *name, DataType type) {
    if (!t->top) return -1;
    /* Check for duplicate in current scope. */
    if (symtab_lookup_current(t, name)) return -1;
    Symbol *sym = calloc(1, sizeof(Symbol));
    strncpy(sym->name, name, 63);
    sym->type        = type;
    sym->scope_level = t->level;
    unsigned int h   = hash_str(name);
    sym->next        = t->top->buckets[h];
    t->top->buckets[h] = sym;
    return 0;
}

Symbol *symtab_lookup(SymbolTable *t, const char *name) {
    for (ScopeFrame *f = t->top; f; f = f->prev) {
        unsigned int h = hash_str(name);
        for (Symbol *s = f->buckets[h]; s; s = s->next)
            if (strcmp(s->name, name) == 0) return s;
    }
    return NULL;
}

Symbol *symtab_lookup_current(SymbolTable *t, const char *name) {
    if (!t->top) return NULL;
    unsigned int h = hash_str(name);
    for (Symbol *s = t->top->buckets[h]; s; s = s->next)
        if (strcmp(s->name, name) == 0) return s;
    return NULL;
}

void symtab_dump(SymbolTable *t) {
    printf("=== Symbol Table Dump ===\n");
    for (ScopeFrame *f = t->top; f; f = f->prev) {
        printf("  Scope level %d:\n", f->level);
        for (int i = 0; i < SYMTAB_BUCKETS; i++) {
            for (Symbol *s = f->buckets[i]; s; s = s->next)
                printf("    %-20s  type=%s\n", s->name,
                       s->type == TYPE_INT ? "int" :
                       s->type == TYPE_FLOAT ? "float" : "unknown");
        }
    }
}
