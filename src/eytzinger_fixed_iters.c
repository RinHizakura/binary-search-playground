#include <math.h>
#include <stdlib.h>
#include "eytzinger.h"

static int iters;
static int max;

int *eytzinger_fixed_iters_prepare(int *src_arr, int n)
{
    // assume the multiplication never overflow
    int *arr = aligned_alloc(64, (n + 1) * sizeof(int));
    eytzinger(src_arr, arr, 0, 1, n);
    arr[0] = -1;

    iters = log2(n + 1);
    max = arr[n - 1];
    return arr;
}

int eytzinger_fixed_iters_lower_bound(int *arr, int n, int val)
{
    int k = 1;

    for (int i = 0; i < iters; i++)
        k = 2 * k + (arr[k] < val);

    int *loc = (k <= n ? arr + k : arr);
    k = 2 * k + (*loc < val);

    k >>= __builtin_ffs(~k);

    return arr[k];
}
