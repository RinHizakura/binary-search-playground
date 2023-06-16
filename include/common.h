#ifndef COMMON_H
#define COMMON_H

#define __ALIGN(x, a) __ALIGN_MASK(x, (typeof(x)) (a) -1)
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN_UP(x, a) __ALIGN((x), (a))

#define DIV_UP(n, d) (((n) + (d) -1) / (d))

#endif
