/*
 * test1.c  —  Module 8: Arithmetic-heavy test program.
 * Purpose: demonstrate constant folding and arithmetic simplification in -O3.
 *
 * Compile and emit LLVM IR:
 *   clang test1.c -S -emit-llvm -o test1.ll
 *   clang test1.c -S -emit-llvm -O3 -o test1_O3.ll
 *
 * Observable -O3 optimisations:
 *   1. Constant folding:   expressions like 3*4+10 evaluated at compile time.
 *   2. Alloca elimination: local variables moved into SSA registers (mem2reg).
 *   3. Dead store removal: stores to vars that are never loaded are dropped.
 */

#include <stdio.h>
#include <math.h>

static double compute(double x) {
    /* These constants will be folded by -O3. */
    double a = 3.0 * 4.0 + 10.0;   /* -> 22.0 */
    double b = a * 2.0 - 5.0;       /* -> 39.0 */
    double c = b / 3.0;             /* -> 13.0 */
    double d = x * x + 2.0 * x + 1.0; /* (x+1)^2 — strength reduction */
    double e = c + d;
    return e;
}

static int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main(void) {
    /* Constant expressions — all foldable at compile time with -O3. */
    int a = 10 + 20;          /* 30 */
    int b = a * 3;            /* 90 */
    int c = b - 15;           /* 75 */
    int d = c / 5;            /* 15 */
    int dead = 42;            /* dead store: never used after this line */
    (void)dead;

    printf("Constant arithmetic results:\n");
    printf("  a = %d\n", a);
    printf("  b = %d\n", b);
    printf("  c = %d\n", c);
    printf("  d = %d\n", d);

    printf("factorial(10) = %d\n", factorial(10));
    printf("compute(5.0)  = %g\n", compute(5.0));

    /* Loop with invariant computation. */
    int sum = 0;
    int loop_inv = 3 * 7;  /* 21 — loop invariant */
    for (int i = 0; i < 100; i++) {
        sum += loop_inv;
    }
    printf("sum (100 * 21) = %d\n", sum);

    return 0;
}
