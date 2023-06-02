#include <stdlib.h>
#include "eytzinger.h"

static int max;

int *eytzinger_simple_prepare(int *src_arr, int n)
{
    max = src_arr[n - 1];

    int *arr = calloc(n + 1, sizeof(int));
    eytzinger(src_arr, arr, 0, 1, n);
    arr[0] = -1;
    return arr;
}

int eytzinger_simple_lower_bound(int *arr, int n, int val)
{
    if (max < val)
        return -1;

    int k = 1;
    while (k <= n)
        k = 2 * k + (arr[k] < val);
    k >>= __builtin_ffs(~k);
    return arr[k];
}
