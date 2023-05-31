#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "bench.h"

#define CHECK_CORRECTNESS
#define SEARCH_TIME 128

int *baseline_prepare(int *src_arr, int n);
int baseline_lower_bound(int *arr, int n, int val);

int *branchless_prepare(int *src_arr, int n);
int branchless_lower_bound(int *arr, int n, int val);

int *prefetch_prepare(int *src_arr, int n);
int prefetch_lower_bound(int *arr, int n, int val);

int *eytzinger_prepare(int *src_arr, int n);
int eytzinger_lower_bound(int *arr, int n, int val);

int *eytzinger_prefetch_prepare(int *src_arr, int n);
int eytzinger_prefetch_lower_bound(int *arr, int n, int val);

int *eytzinger_optimized_prepare(int *src_arr, int n);
int eytzinger_optimized_lower_bound(int *arr, int n, int val);

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

static bool bench(int *src_arr,
                  int n,
                  PREPARE_FUNC prepare,
                  LOWER_BOUND_FUNC lower_bound)
{
    bool result = true;
    int *ret = prepare(src_arr, n);

    int *arr;
    if (ret == NULL)
        arr = src_arr;
    else
        arr = ret;

    for (int i = 0; i < SEARCH_TIME; i++) {
        int val = rand();
        int ans = lower_bound(arr, n, val);
#ifdef CHECK_CORRECTNESS
        // Use the result of baseline binary search as correct answer
        int expect = baseline_lower_bound(src_arr, n, val);
        if (expect != ans) {
            printf("%d != %d\n", expect, ans);
            result = false;
        }
#endif
    }

    if (ret != NULL)
        free(arr);

    return result;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: bench <the size of test array>\n");
        return -1;
    }

    size_t sz = 0;
    sscanf(argv[1], "%ld", &sz);

    int *src_arr = generate_input(sz);

    func_t f[] = {
        {baseline_prepare, baseline_lower_bound, "baseline"},
        {branchless_prepare, branchless_lower_bound, "branchless"},
        {prefetch_prepare, prefetch_lower_bound, "prefetch"},
        {eytzinger_prepare, eytzinger_lower_bound, "eytzinger"},
        {eytzinger_prefetch_prepare, eytzinger_prefetch_lower_bound,
         "eytzinger_prefetch"},
        {eytzinger_optimized_prepare, eytzinger_optimized_lower_bound,
         "eytzinger_optimized"},
    };

    for (int i = 0; i < 6; i++) {
        bool result = bench(src_arr, sz, f[i].prepare, f[i].lower_bound);
        printf("The result of %s is %s\n", f[i].name,
               result ? "TRUE" : "FALSE");
    }

    free(src_arr);

    return 0;
}
