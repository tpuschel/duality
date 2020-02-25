/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_OVERFLOW_H
#define DY_OVERFLOW_H

#include <duality/support/api.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

DY_SUPPORT_API bool dy_sadd_overflow(int a, int b, int *c);
DY_SUPPORT_API bool dy_saddl_overflow(long a, long b, long *c);
DY_SUPPORT_API bool dy_saddll_overflow(long long a, long long b, long long *c);
DY_SUPPORT_API bool dy_uadd_overflow(unsigned a, unsigned b, unsigned *c);
DY_SUPPORT_API bool dy_uaddl_overflow(unsigned long a, unsigned long b, unsigned long *c);
DY_SUPPORT_API bool dy_uaddll_overflow(unsigned long long a, unsigned long long b, unsigned long long *c);

DY_SUPPORT_API bool dy_size_t_add_overflow(size_t a, size_t b, size_t *c);

DY_SUPPORT_API bool dy_intmax_t_add_overflow(intmax_t a, intmax_t b, intmax_t *c);

DY_SUPPORT_API bool dy_smul_overflow(int a, int b, int *c);
DY_SUPPORT_API bool dy_smull_overflow(long a, long b, long *c);
DY_SUPPORT_API bool dy_smulll_overflow(long long a, long long b, long long *c);
DY_SUPPORT_API bool dy_umul_overflow(unsigned a, unsigned b, unsigned *c);
DY_SUPPORT_API bool dy_umull_overflow(unsigned long a, unsigned long b, unsigned long *c);
DY_SUPPORT_API bool dy_umulll_overflow(unsigned long long a, unsigned long long b, unsigned long long *c);

DY_SUPPORT_API bool dy_size_t_mul_overflow(size_t a, size_t b, size_t *c);

DY_SUPPORT_API bool dy_intmax_t_mul_overflow(intmax_t a, intmax_t b, intmax_t *c);

#endif // DY_OVERFLOW_H
