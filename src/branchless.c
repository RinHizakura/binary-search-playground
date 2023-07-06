#include <stddef.h>

int *branchless_prepare(int *src_arr, int n)
{
    // do nothing, we don't need any preparation in this algorithm
    return src_arr;
}

int branchless_lower_bound(int *arr, int n, int val)
{
    if (arr[n - 1] < val)
        return -1;

    int *base = arr, len = n;
    while (len > 1) {
        int half = len / 2;
        base +=
            (base[half - 1] < val) * half;  // will be replaced with a "cmov"
        len -= half;
    }
    return *base;
}
