#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bench.h"

#define TOTAL 10

#define __DECLARE_FUNC(f_sig, clean_f)                 \
    int *f_sig##_prepare(int *src_arr, int n);         \
    int f_sig##_lower_bound(int *arr, int n, int val); \
    const func_t f_sig##_f = {                         \
        .prepare = f_sig##_prepare,                    \
        .lower_bound = f_sig##_lower_bound,            \
        .clean = clean_f,                              \
        .name = #f_sig,                                \
    };

#define DECLARE_FUNC(f_sig) __DECLARE_FUNC(f_sig, NULL)

#define DECLARE_FUNC_WITH_CLEAN(f_sig) \
    void f_sig##_clean(int *arr);      \
    __DECLARE_FUNC(f_sig, f_sig##_clean)

DECLARE_FUNC(baseline);
DECLARE_FUNC(branchless);
DECLARE_FUNC(prefetch);
DECLARE_FUNC(shar);
DECLARE_FUNC_WITH_CLEAN(eytzinger_simple);
DECLARE_FUNC_WITH_CLEAN(eytzinger_prefetch);
DECLARE_FUNC_WITH_CLEAN(eytzinger_fixed_iters);
DECLARE_FUNC_WITH_CLEAN(b_tree_simple);
DECLARE_FUNC_WITH_CLEAN(b_tree_optimized);
DECLARE_FUNC_WITH_CLEAN(b_plus_tree);

const func_t f[TOTAL] = {
    baseline_f,
    branchless_f,
    prefetch_f,
    shar_f,
    eytzinger_simple_f,
    eytzinger_prefetch_f,
    eytzinger_fixed_iters_f,
    b_tree_simple_f,
    b_tree_optimized_f,
    b_plus_tree_f,
};

static int cmp(const void *a, const void *b)
{
    return *(int *) a - *(int *) b;
}

static int *generate_input(size_t sz)
{
    int *src_arr = malloc(sz * sizeof(int));
    for (int i = 0; i < sz; i++) {
        src_arr[i] = rand();
    }
    qsort(src_arr, sz, sizeof(int), cmp);

    return src_arr;
}

static int *generate_query(size_t sz)
{
    int *src_arr = malloc(sz * sizeof(int));
    for (int i = 0; i < sz; i++) {
        src_arr[i] = rand();
    }

    return src_arr;
}

static long bench(int *src_arr,
                  int *query_arr,
                  int n,
                  int m,
                  PREPARE_FUNC prepare,
                  LOWER_BOUND_FUNC lower_bound,
                  CLEAN_FUNC clean)
{
    int *arr = prepare(src_arr, n);

    struct timespec tt1, tt2;
    clock_gettime(CLOCK_MONOTONIC, &tt1);
    for (int i = 0; i < m; i++) {
        int ans = lower_bound(arr, n, query_arr[i]);
#ifdef CHECK_CORRECTNESS
        // Use the result of baseline binary search as correct answer
        int expect = baseline_lower_bound(src_arr, n, query_arr[i]);
        if (expect != ans) {
            printf("[WARNING] Key = %d: %d != %d\n", query_arr[i], expect, ans);
        }
#endif
    }
    clock_gettime(CLOCK_MONOTONIC, &tt2);

    long time = (long long) (tt2.tv_sec * 1e9 + tt2.tv_nsec) -
                (long long) (tt1.tv_sec * 1e9 + tt1.tv_nsec);

    if (clean != NULL)
        clean(arr);

    return time;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("usage: bench <the size of test array> <the time of query>\n");
        return -1;
    }

    // ALSR
    srand((uintptr_t) &argc);

    size_t n = 0;
    size_t m = 0;
    sscanf(argv[1], "%ld", &n);
    sscanf(argv[2], "%ld", &m);

    int *src_arr = generate_input(n);
    int *query_arr = generate_query(m);

    for (int i = 0; i < TOTAL; i++) {
        printf("%s, ", f[i].name);
    }
    puts("");

    for (int i = 0; i < TOTAL; i++) {
        long time = bench(src_arr, query_arr, n, m, f[i].prepare,
                          f[i].lower_bound, f[i].clean);
        printf("%ld, ", time);
    }
    puts("");

    free(src_arr);
    free(query_arr);

    return 0;
}
