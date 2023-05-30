int eytzinger(int *src_arr, int *arr, int i, int k, int n)
{
    if (k <= n) {
        i = eytzinger(src_arr, arr, i, 2 * k, n);
        arr[k] = src_arr[i++];
        i = eytzinger(src_arr, arr, i, 2 * k + 1, n);
    }
    return i;
}
