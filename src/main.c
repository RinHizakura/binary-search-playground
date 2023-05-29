#include <stdio.h>
#include <stdlib.h>
#include "bench.h"

#define SEARCH_TIME 128

int *baseline_prepare(int *src_arr, int n);
int baseline_lower_bound(int *arr, int n, int val);

static void bench(int *src_arr,
                  int n,
                  PREPARE_FUNC prepare,
                  LOWER_BOUND_FUNC lower_bound)
{
    int *ret = prepare(src_arr, n);

    int *arr;
    if (ret == NULL) {
        arr = src_arr;
    } else {
        arr = ret;
    }

    for (int i = 0; i < SEARCH_TIME; i++) {
        int val = rand();
        int ans = lower_bound(arr, n, val);
        printf("The lower_bound of %d is %d\n", val, ans);
    }

    if (ret != NULL)
        free(arr);
}

static int cmp(const void *a, const void *b)
{
    return *(int *) a - *(int *) b;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: bench <the size of test array>\n");
        return -1;
    }

    // Genereate benchmark array
    size_t sz = 0;
    sscanf(argv[1], "%ld", &sz);

    int *src_arr = malloc(sz * sizeof(int));
    for (int i = 0; i < sz; i++) {
        src_arr[i] = rand();
    }
    qsort(src_arr, sz, sizeof(int), cmp);

    func_t f[1] = {{baseline_prepare, baseline_lower_bound}};

    for (int i = 0; i < 1; i++) {
        bench(src_arr, sz, f[i].prepare, f[i].lower_bound);
    }

    free(src_arr);

    return 0;
}
