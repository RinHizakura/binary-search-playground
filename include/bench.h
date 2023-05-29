#ifndef BENCH_H
#define BENCH_H

typedef int (*LOWER_BOUND_FUNC)(int *, int, int);
typedef int *(*PREPARE_FUNC)(int *, int);

typedef struct {
    PREPARE_FUNC prepare;
    LOWER_BOUND_FUNC lower_bound;
} func_t;

#endif
