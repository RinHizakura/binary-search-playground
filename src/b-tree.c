#include <limits.h>
#include <stdlib.h>
#include "common.h"

const int B = 16;
static int nblocks;
static int max;

#define KEY(btree, k, i) btree[k * B + i]

// Get the node index for the child i of node k
static inline int go(int k, int i)
{
    return k * (B + 1) + i + 1;
}

static void build(int *src_arr, int *btree, int k, int n)
{
    static int t = 0;
    if (k < nblocks) {
        for (int i = 0; i < B; i++) {
            build(src_arr, btree, go(k, i), n);
            KEY(btree, k, i) = (t < n ? src_arr[t++] : max);
        }
        build(src_arr, btree, go(k, B), n);
    }
}

int *b_tree_prepare(int *src_arr, int n)
{
    max = src_arr[n - 1];
    nblocks = ALIGN_UP(n, B);
    /* This is actually a two dimensional array (int btree[nblocks][B]),
     * but we allocate as a one dimensional array to make sure that we free
     * the space correctly in the bench function. */
    int *btree = aligned_alloc(64, nblocks * B * sizeof(int));

    build(src_arr, btree, 0, n);

    return btree;
}

/* This function compare each key in the target node k, and
 * set the corresponing bit if the key >= val.
 *
 * TODO: These can be implemented by SIMD instruction */
static int cmp(int *btree, int k, int val)
{
    int mask = (1 << B);

    for (int i = 0; i < B; i++)
        mask |= (KEY(btree, k, i) >= val) << i;

    return mask;
}

int b_tree_lower_bound(int *btree, int n, int val)
{
    if (max < val)
        return -1;

    int k = 0, res = max;
    while (k < nblocks) {
        int cmp_mask = cmp(btree, k, val);
        int i = __builtin_ffs(cmp_mask) - 1;
        if (i < B)
            res = KEY(btree, k, i);
        k = go(k, i);
    }
    return res;
}
