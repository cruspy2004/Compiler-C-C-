/*
 * postfix_eval.c  —  Module 1 Task 2
 * Standalone postfix expression evaluator.
 *
 * Reads a postfix expression from stdin (tokens separated by spaces).
 * Operands: non-negative integers.
 * Operators: +  -  *
 * Prints stack state after every push/pop, then the final result.
 *
 * Example:  echo "3 4 + 2 *" | ./postfix_eval
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STACK 256

static int stack[MAX_STACK];
static int top = -1;

static void print_stack(void) {
    printf("[");
    for (int i = 0; i <= top; i++) {
        printf("%d", stack[i]);
        if (i < top) printf(", ");
    }
    printf("]");
}

static void push(int val) {
    if (top >= MAX_STACK - 1) {
        fprintf(stderr, "Stack overflow\n");
        exit(1);
    }
    stack[++top] = val;
    printf("PUSH %d  ->  ", val);
    print_stack();
    printf("\n");
}

static int pop(void) {
    if (top < 0) {
        fprintf(stderr, "Stack underflow — not enough operands\n");
        exit(1);
    }
    return stack[top--];
}

int main(void) {
    char token[64];
    while (scanf("%63s", token) == 1) {
        if (isdigit((unsigned char)token[0]) ||
            (token[0] == '-' && isdigit((unsigned char)token[1]))) {
            push(atoi(token));
        } else if (strlen(token) == 1 &&
                   (token[0] == '+' || token[0] == '-' || token[0] == '*')) {
            int b = pop();
            int a = pop();
            int result;
            switch (token[0]) {
                case '+': result = a + b; break;
                case '-': result = a - b; break;
                case '*': result = a * b; break;
                default:  result = 0;
            }
            printf("POP %d, %d  ->  %d %c %d = %d\n", a, b, a, token[0], b, result);
            push(result);
        } else {
            fprintf(stderr, "Unknown token: %s\n", token);
            exit(1);
        }
    }

    if (top != 0) {
        fprintf(stderr, "Error: %d value(s) left on stack (malformed expression)\n", top + 1);
        return 1;
    }
    printf("\nResult: %d\n", stack[0]);
    return 0;
}
