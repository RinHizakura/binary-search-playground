#include <string.h>
#include <sys/mman.h>
#include "b-tree.h"
#include "common.h"

#ifdef __AVX2__
#include <immintrin.h>
#endif

#define HUGE_PAGESIZE (1 << 21)

static int max;
static int H;
static int S;

#define BLOCKS(n) (ALIGN_UP(n, B) / B)

#include <stdio.h>

/* For n key values, count the number of keys on the previous
 * layer. The calculation comes from the fact that for a group
 * of B + 1 child node, one node with B keys will be needed. */
static int prev_keys(int n)
{
    return (BLOCKS(n) + B) / (B + 1) * B;
}

static int height(int n)
{
    return (n <= B ? 1 : height(prev_keys(n)) + 1);
}

/* For the tree with n keys, get the offset set of the starts
 * oflayer h starts (layer 0 is the largest and the leaf node) */
static int offset(int n, int h)
{
    int k = 0;
    while (h--) {
        k += BLOCKS(n) * B;
        n = prev_keys(n);
    }
    return k;
}

static void permute(int *node)
{
    const __m256i perm = _mm256_setr_epi32(4, 5, 6, 7, 0, 1, 2, 3);
    __m256i *middle = (__m256i *) (node + 4);
    __m256i x = _mm256_loadu_si256(middle);
    x = _mm256_permutevar8x32_epi32(x, perm);
    _mm256_storeu_si256(middle, x);
}

int *b_plus_tree_prepare(int *src_arr, int n)
{
    max = src_arr[n - 1];

    /* This must be initialized first because we'll use
     * it for prepare later. */
    H = height(n);
    // The size of tree will equal to the start offset of (non-existent) layer H
    S = offset(n, H);

    int sz = ALIGN_UP(sizeof(int) * S, HUGE_PAGESIZE);
    int *btree = aligned_alloc(HUGE_PAGESIZE, sz);
    madvise(btree, sz, MADV_HUGEPAGE);

    memcpy(btree, src_arr, sizeof(int) * n);
    for (int i = n; i < S; i++)
        btree[i] = max;

    /* Construct the tree layer by layer */
    for (int h = 1; h < H; h++) {
        int offset_cur = offset(n, h);
        int offset_range = offset(n, h + 1) - offset_cur;
        for (int i = 0; i < offset_range; i++) {
            /* k is the index of node */
            int k = i / B;
            /* j will range from 0 - 15, which is the sub-index of
             * the corresponding node */
            int j = i - k * B;

            /* Descend to the offset of the right node */
            k = k * (B + 1) + j + 1;
            /* and then go left until reaching the leaf */
            for (int l = 0; l < h - 1; l++)
                k *= (B + 1);
            // The smallest key on the leaf will the key
            btree[offset_cur + i] = (k * B < n ? btree[k * B] : max);
        }
    }

    // Don't permute layer 0 to avoid index translation on it
    for (int i = offset(n, 1); i < S; i += B)
        permute(btree + i);

    return btree;
}

static int permuted_rank(int *btree, int k, __m256i x_vec)
{
    __m256i a = _mm256_load_si256((__m256i *) &btree[k]);
    __m256i b = _mm256_load_si256((__m256i *) &btree[k + 8]);

    __m256i mask_a = _mm256_cmpgt_epi32(a, x_vec);
    __m256i mask_b = _mm256_cmpgt_epi32(b, x_vec);

    __m256i mask_vec = _mm256_packs_epi32(mask_a, mask_b);
    int mask = _mm256_movemask_epi8(mask_vec);

    return __builtin_ctz(mask) >> 1;
}

static int direct_rank(int *btree, int k, __m256i x_vec)
{
    __m256i a = _mm256_load_si256((__m256i *) &btree[k]);
    __m256i b = _mm256_load_si256((__m256i *) &btree[k + 8]);

    __m256i ca = _mm256_cmpgt_epi32(a, x_vec);
    __m256i cb = _mm256_cmpgt_epi32(b, x_vec);

    int lower = _mm256_movemask_ps((__m256) ca);
    int upper = _mm256_movemask_ps((__m256) cb);

    int mask = (1 << 16) | (upper << 8) | lower;

    return __builtin_ctz(mask);
}

int b_plus_tree_lower_bound(int *btree, int n, int val)
{
    if (max < val)
        return -1;

    int k = 0;
    __m256i x_vec = _mm256_set1_epi32(val - 1);

    for (int h = H - 1; h > 0; h--) {
        int i = permuted_rank(btree, offset(n, h) + k, x_vec);

        k = k * (B + 1) + i * B;
    }
    int i = direct_rank(btree, k, x_vec);
    return btree[k + i];
}
