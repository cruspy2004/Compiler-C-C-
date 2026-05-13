#ifndef TOKEN_H
#define TOKEN_H

/* Token types shared between lexer and parser. */
typedef enum {
    TOK_INT_LIT = 256,
    TOK_FLOAT_LIT,
    TOK_IDENT,
    TOK_INT,
    TOK_FLOAT,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_RETURN,
    TOK_THEN,
    TOK_BEGIN,
    TOK_END,
    TOK_PROCEDURE,
    TOK_FUNCTION,
    TOK_EQ,        /* == */
    TOK_NEQ,       /* != */
    TOK_EOF
} TokenType;

#endif /* TOKEN_H */
