#include <stdlib.h>
#include "eytzinger_base.h"

int *eytzinger_prefetch_prepare(int *src_arr, int n)
{
    // assume the multiplication never overflow
    int *arr = aligned_alloc(64, (n + 1) * sizeof(int));
    eytzinger(src_arr, arr, 0, 1, n);
    return arr;
}

int eytzinger_prefetch_lower_bound(int *arr, int n, int val)
{
    int k = 1;
    while (k <= n) {
        __builtin_prefetch(arr + k * 16);
        k = 2 * k + (arr[k] < val);
    }
    k >>= __builtin_ffs(~k);
    return arr[k];
}
