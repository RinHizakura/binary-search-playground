#include <stddef.h>

int *baseline_prepare(int *src_arr, int n)
{
    // do nothing, we don't need any preparation in this algorithm
    return NULL;
}

int baseline_lower_bound(int *arr, int n, int val)
{
    int l = 0, r = n - 1;
    while (l < r) {
        int t = (l + r) / 2;

        if (arr[t] >= val)
            r = t;
        else
            l = t + 1;
    }
    return arr[l];
}