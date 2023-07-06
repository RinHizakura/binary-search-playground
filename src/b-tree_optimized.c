#include <limits.h>
#include <sys/mman.h>
#include "b-tree.h"
#include "common.h"

#ifdef __AVX2__
#include <immintrin.h>
#endif

#define HUGE_PAGESIZE (1 << 21)

static int H;
static int nblocks;
static int max;

static int height(int n)
{
    // grow the tree until its size exceeds n elements
    int s = 0,  // total size so far
        l = B,  // size of the next layer
        h = 0;  // height so far
    while (s + l - B < n) {
        s += l;
        l *= (B + 1);
        h++;
    }
    return h;
}

static void permute(int *node)
{
    const __m256i perm = _mm256_setr_epi32(4, 5, 6, 7, 0, 1, 2, 3);
    // Move value of btree[k][4]/btree[k][8 + 4] to a destination vector
    __m256i *middle = (__m256i *) (node + 4);
    __m256i x = _mm256_loadu_si256(middle);

    /* This will let the node
     * [0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15]
     * being [0 | 1 | 2 | 3 | 8 | 9 | 10 | 11 | 4 | 5 | 6 | 7 | 12 | 13 | 14 |
     * 15] */
    x = _mm256_permutevar8x32_epi32(x, perm);
    _mm256_storeu_si256(middle, x);
}

static void build(int *src_arr, int *btree, int k, int n)
{
    static int t = 0;
    if (k < nblocks) {
        for (int i = 0; i < B; i++) {
            build(src_arr, btree, go(k, i), n);
            KEY(btree, k, i) = (t < n ? src_arr[t++] : max);
        }
        permute(&KEY(btree, k, 0));
        build(src_arr, btree, go(k, B), n);
    }
}

int *b_tree_optimized_prepare(int *src_arr, int n)
{
    H = height(n);
    max = src_arr[n - 1];
    nblocks = ALIGN_UP(n, B) / B;

    int sz = ALIGN_UP(nblocks * B * sizeof(int), HUGE_PAGESIZE);
    int *btree = aligned_alloc(HUGE_PAGESIZE, sz);
    madvise(btree, sz, MADV_HUGEPAGE);
    build(src_arr, btree, 0, n);

    return btree;
}

static int rank(int *btree, int k, __m256i x_vec)
{
    __m256i a = _mm256_load_si256((__m256i *) &KEY(btree, k, 0));
    __m256i b = _mm256_load_si256((__m256i *) &KEY(btree, k, 8));

    __m256i mask_a = _mm256_cmpgt_epi32(a, x_vec);
    __m256i mask_b = _mm256_cmpgt_epi32(b, x_vec);

    /* The result of comparison mask_a([0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 ]) and
     * mask_b([8 | 9 | 10 | 11 | 12 | 13 | 14 | 15]) will be packed to
     * [0 | 1 | 2 | 3 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15] */
    __m256i mask_vec = _mm256_packs_epi32(mask_a, mask_b);
    int mask = _mm256_movemask_epi8(mask_vec);

    // we need to divide the result by two because we call movemask_epi8 on
    // 16-bit masks:
    return __builtin_ctz(mask) >> 1;
}

const int translate[16] = {0, 1, 2, 3, 8,  9,  10, 11,
                           4, 5, 6, 7, 12, 13, 14, 15};
static int update(int res, int *btree, int k, int i)
{
    if (i >= B) {
        return res;
    }
    return KEY(btree, k, translate[i]);
}

int b_tree_optimized_lower_bound(int *btree, int n, int val)
{
    if (max < val)
        return -1;

    __m256i x_vec = _mm256_set1_epi32(val - 1);
    int k = 0, res = INT_MAX;
    for (int h = 0; h < H; h++) {
        int i = rank(btree, k, x_vec);
        res = update(res, btree, k, i);
        k = go(k, i);
    }
    // the last branch:
    if (k < nblocks) {
        int i = rank(btree, k, x_vec);
        res = update(res, btree, k, i);
    }
    return res;
}

void b_tree_optimized_clean(int *btree)
{
    if (btree) {
        free(btree);
    }
}
