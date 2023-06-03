#include <limits.h>
#include <stdlib.h>
#include "b-tree.h"
#include "common.h"

#ifdef __AVX2__
#include <immintrin.h>
#endif

static int nblocks;
static int max;

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

int *b_tree_simple_prepare(int *src_arr, int n)
{
    max = src_arr[n - 1];
    nblocks = ALIGN_UP(n, B) / B;
    /* This is actually a two dimensional array (int btree[nblocks][B]),
     * but we allocate as a one dimensional array to make sure that we free
     * the space correctly in the bench function. */
    int *btree = aligned_alloc(64, nblocks * B * sizeof(int));

    build(src_arr, btree, 0, n);

    return btree;
}

/* This function compare each key in the target node k, and
 * set the corresponing bit if the key >= val. */
static int cmp(int *btree, int k, int val)
{
    int mask;
#ifdef __AVX2__
    __m256i x_vec, y_vec, mask_vec;

    x_vec = _mm256_set1_epi32(val);
    y_vec = _mm256_load_si256((__m256i *) &KEY(btree, k, 0));
    mask_vec = _mm256_cmpgt_epi32(x_vec, y_vec);
    int lower = _mm256_movemask_ps((__m256) mask_vec);

    y_vec = _mm256_load_si256((__m256i *) &KEY(btree, k, 8));
    mask_vec = _mm256_cmpgt_epi32(x_vec, y_vec);
    int upper = _mm256_movemask_ps((__m256) mask_vec);

    mask = ~(lower | upper << 8);
#else
    mask = (1 << B);

    for (int i = 0; i < B; i++)
        mask |= (KEY(btree, k, i) >= val) << i;
#endif
    return mask;
}

int b_tree_simple_lower_bound(int *btree, int n, int val)
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
