#include <stddef.h>

int *prefetch_prepare(int *src_arr, int n)
{
    // do nothing, we don't need any preparation in this algorithm
    return src_arr;
}

int prefetch_lower_bound(int *arr, int n, int val)
{
    if (arr[n - 1] < val)
        return -1;

    int *base = arr, len = n;
    while (len > 1) {
        int half = len / 2;
        len -= half;
        __builtin_prefetch(&base[len / 2 - 1]);
        __builtin_prefetch(&base[half + len / 2 - 1]);
        base += (base[half - 1] < val) * half;
    }
    return *base;
}

void prefetch_clean(int *src_arr) {}
