#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bench.h"

int *baseline_prepare(int *src_arr, int n);
int baseline_lower_bound(int *arr, int n, int val);

int *branchless_prepare(int *src_arr, int n);
int branchless_lower_bound(int *arr, int n, int val);

int *prefetch_prepare(int *src_arr, int n);
int prefetch_lower_bound(int *arr, int n, int val);

int *shar_prepare(int *src_arr, int n);
int shar_lower_bound(int *arr, int n, int val);

int *eytzinger_simple_prepare(int *src_arr, int n);
int eytzinger_simple_lower_bound(int *arr, int n, int val);

int *eytzinger_prefetch_prepare(int *src_arr, int n);
int eytzinger_prefetch_lower_bound(int *arr, int n, int val);

int *eytzinger_fixed_iters_prepare(int *src_arr, int n);
int eytzinger_fixed_iters_lower_bound(int *arr, int n, int val);

int *b_tree_simple_prepare(int *src_arr, int n);
int b_tree_simple_lower_bound(int *arr, int n, int val);

int *b_tree_optimized_prepare(int *src_arr, int n);
int b_tree_optimized_lower_bound(int *arr, int n, int val);

int *b_plus_tree_prepare(int *src_arr, int n);
int b_plus_tree_lower_bound(int *btree, int n, int val);

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
                  LOWER_BOUND_FUNC lower_bound)
{
    int *ret = prepare(src_arr, n);

    int *arr;
    if (ret == NULL)
        arr = src_arr;
    else
        arr = ret;

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

    if (ret != NULL)
        free(arr);

    return time;
}

#define BASELINE 0
#define BRANCHLESS 1
#define PREFETCH 2
#define SHAR 3
#define EYTZINGER_SIMPLE 4
#define EYTZINGER_PREFETCH 5
#define EYTZINGER_FIXED_ITERS 6
#define BTREE_SIMPLE 7
#define BTREE_OPT 8
#define B_PLUS 9
#define TOTAL 10

const func_t baseline_f = {
    .prepare = baseline_prepare,
    .lower_bound = baseline_lower_bound,
    .name = "baseline",
};

const func_t branchless_f = {
    .prepare = branchless_prepare,
    .lower_bound = branchless_lower_bound,
    .name = "branchless",
};

const func_t prefetch_f = {
    .prepare = prefetch_prepare,
    .lower_bound = prefetch_lower_bound,
    .name = "prefetch",
};

const func_t shar_f = {
    .prepare = shar_prepare,
    .lower_bound = shar_lower_bound,
    .name = "shar",
};

const func_t eytzinger_simple_f = {
    .prepare = eytzinger_simple_prepare,
    .lower_bound = eytzinger_simple_lower_bound,
    .name = "eytzinger_simple",
};

const func_t eytzinger_prefetch_f = {
    .prepare = eytzinger_prefetch_prepare,
    .lower_bound = eytzinger_prefetch_lower_bound,
    .name = "eytzinger_prefetch",
};

const func_t eytzinger_fixed_iters_f = {
    .prepare = eytzinger_fixed_iters_prepare,
    .lower_bound = eytzinger_fixed_iters_lower_bound,
    .name = "eytzinger_fixed_iters",
};

const func_t b_tree_simple_f = {.prepare = b_tree_simple_prepare,
                                .lower_bound = b_tree_simple_lower_bound,
                                .name = "B-tree_simple"};

const func_t b_tree_optimized_f = {
    .prepare = b_tree_optimized_prepare,
    .lower_bound = b_tree_optimized_lower_bound,
    .name = "B-tree_optimized",
};

const func_t b_plus_tree_f = {.prepare = b_plus_tree_prepare,
                              .lower_bound = b_plus_tree_lower_bound,
                              .name = "B+tree"};

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
        long time =
            bench(src_arr, query_arr, n, m, f[i].prepare, f[i].lower_bound);
        printf("The result of %-25s is %ld(ns)\n", f[i].name, time);
    }

    free(src_arr);
    free(query_arr);

    return 0;
}
