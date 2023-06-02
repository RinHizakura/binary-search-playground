#include <stddef.h>

int *shar_prepare(int *src_arr, int n)
{
    return NULL;
}

int shar_lower_bound(int *arr, int n, int val)
{
    // Consider the special case at first
    if (arr[n - 1] < val)
        return -1;

    if (arr[0] > val)
        return arr[0];

    if (n == 1)
        return (arr[0] <= val) ? arr[0] : -1;

    // assume n won't be 0
    int u = 32 - __builtin_clz(2 * n - 1) - 2;
    // Let p being 2^floor(lgn)
    int p = 1 << (u);

    int i = (arr[p] <= val) * (n - p);

    while (p >>= 1) {
        if (arr[i + p] <= val)
            i += p;
    }

    return arr[i + 1];
}
