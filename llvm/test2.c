/*
 * test2.c  —  Module 8: Loop-heavy test program.
 * Purpose: demonstrate loop unrolling, LICM, and strength reduction in -O3.
 *
 * Compile:
 *   clang test2.c -S -emit-llvm -o test2.ll
 *   clang test2.c -S -emit-llvm -O3 -o test2_O3.ll
 *
 * Observable -O3 optimisations:
 *   1. LICM:        invariant computations moved outside the loop body.
 *   2. Loop unrolling / vectorisation: the inner loop may be unrolled or
 *      replaced by a vector add instruction.
 *   3. Strength reduction: repeated addition replaced by multiplication.
 *   4. Dead code:   unused accumulator variables dropped.
 */

#include <stdio.h>

#define N 1000

static int dot_product(const int *a, const int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += a[i] * b[i];
    return sum;
}

static void fill(int *arr, int n, int val) {
    for (int i = 0; i < n; i++)
        arr[i] = val;
}

int main(void) {
    int a[N], b[N];
    fill(a, N, 3);
    fill(b, N, 5);

    /* Loop with an invariant subexpression. */
    int total = 0;
    int scale = 2 * 3;   /* 6 — loop invariant: LICM hoists this */
    for (int i = 0; i < N; i++) {
        total += a[i] * scale;   /* scale is invariant */
    }
    printf("total (N*3*6) = %d\n", total);

    /* Nested loop: inner is unrollable. */
    long long mat_sum = 0;
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < N; j++)
            mat_sum += (long long)i * j;
    printf("mat_sum = %lld\n", mat_sum);

    /* Dot product. */
    printf("dot(a,b) = %d\n", dot_product(a, b, N));

    /* Strength reduction: i*4 may become shift or accumulated add. */
    int sr_sum = 0;
    for (int i = 0; i < N; i++)
        sr_sum += i * 4;
    printf("sr_sum = %d\n", sr_sum);

    return 0;
}
