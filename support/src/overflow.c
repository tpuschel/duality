/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/support/overflow.h>

#include <limits.h>
#include <stdint.h>

bool dy_sadd_overflow(int a, int b, int *c)
{
    if (((b > 0) && (a > (INT_MAX - b))) || ((b < 0) && (a < (INT_MIN - b)))) {
        return true;
    } else {
        *c = a + b;
        return false;
    }
}

bool dy_saddl_overflow(long a, long b, long *c)
{
    if (((b > 0) && (a > (LONG_MAX - b))) || ((b < 0) && (a < (LONG_MIN - b)))) {
        return true;
    } else {
        *c = a + b;
        return false;
    }
}

bool dy_saddll_overflow(long long a, long long b, long long *c)
{
    if (((b > 0) && (a > (LLONG_MAX - b))) || ((b < 0) && (a < (LLONG_MIN - b)))) {
        return true;
    } else {
        *c = a + b;
        return false;
    }
}

bool dy_intmax_t_add_overflow(intmax_t a, intmax_t b, intmax_t *c)
{
    if (((b > 0) && (a > (INTMAX_MAX - b))) || ((b < 0) && (a < (INTMAX_MIN - b)))) {
        return true;
    } else {
        *c = a + b;
        return false;
    }
}

bool dy_uadd_overflow(unsigned a, unsigned b, unsigned *c)
{
    unsigned ret = a + b;
    if (ret < a) {
        return true;
    } else {
        *c = ret;
        return false;
    }
}

bool dy_uaddl_overflow(unsigned long a, unsigned long b, unsigned long *c)
{
    unsigned long ret = a + b;
    if (ret < a) {
        return true;
    } else {
        *c = ret;
        return false;
    }
}

bool dy_uaddll_overflow(unsigned long long a, unsigned long long b, unsigned long long *c)
{
    unsigned long long ret = a + b;
    if (ret < a) {
        return true;
    } else {
        *c = ret;
        return false;
    }
}

bool dy_smul_overflow(int a, int b, int *c)
{
    if (a > 0) {
        if (b > 0) {
            if (a > (INT_MAX / b)) {
                return true;
            }
        } else {
            if (b < (INT_MIN / a)) {
                return true;
            }
        }
    } else {
        if (b > 0) {
            if (a < (INT_MIN / b)) {
                return true;
            }
        } else {
            if ((a != 0) && (b < (INT_MAX / a))) {
                return true;
            }
        }
    }

    *c = a * b;

    return false;
}

bool dy_smull_overflow(long a, long b, long *c)
{
    if (a > 0) {
        if (b > 0) {
            if (a > (LONG_MAX / b)) {
                return true;
            }
        } else {
            if (b < (LONG_MIN / a)) {
                return true;
            }
        }
    } else {
        if (b > 0) {
            if (a < (LONG_MIN / b)) {
                return true;
            }
        } else {
            if ((a != 0) && (b < (LONG_MAX / a))) {
                return true;
            }
        }
    }

    *c = a * b;

    return false;
}

bool dy_smulll_overflow(long long a, long long b, long long *c)
{
    if (a > 0) {
        if (b > 0) {
            if (a > (LLONG_MAX / b)) {
                return true;
            }
        } else {
            if (b < (LLONG_MIN / a)) {
                return true;
            }
        }
    } else {
        if (b > 0) {
            if (a < (LLONG_MIN / b)) {
                return true;
            }
        } else {
            if ((a != 0) && (b < (LLONG_MAX / a))) {
                return true;
            }
        }
    }

    *c = a * b;

    return false;
}

bool dy_intmax_t_mul_overflow(intmax_t a, intmax_t b, intmax_t *c)
{
    if (a > 0) {
        if (b > 0) {
            if (a > (INTMAX_MAX / b)) {
                return true;
            }
        } else {
            if (b < (INTMAX_MIN / a)) {
                return true;
            }
        }
    } else {
        if (b > 0) {
            if (a < (INTMAX_MIN / b)) {
                return true;
            }
        } else {
            if ((a != 0) && (b < (INTMAX_MAX / a))) {
                return true;
            }
        }
    }

    *c = a * b;

    return false;
}

bool dy_umul_overflow(unsigned a, unsigned b, unsigned *c)
{
    if (b == 0) {
        *c = 0;
        return false;
    }

    if (a > UINT_MAX / b) {
        return true;
    }

    *c = a * b;
    return false;
}

bool dy_umull_overflow(unsigned long a, unsigned long b, unsigned long *c)
{
    if (b == 0) {
        *c = 0;
        return false;
    }

    if (a > ULONG_MAX / b) {
        return true;
    }

    *c = a * b;
    return false;
}

bool dy_umulll_overflow(unsigned long long a, unsigned long long b, unsigned long long *c)
{
    if (b == 0) {
        *c = 0;
        return false;
    }

    if (a > ULLONG_MAX / b) {
        return true;
    }

    *c = a * b;
    return false;
}

bool dy_size_t_add_overflow(size_t a, size_t b, size_t *c)
{
    size_t ret = a + b;
    if (ret < a) {
        return true;
    } else {
        *c = ret;
        return false;
    }
}

bool dy_size_t_mul_overflow(size_t a, size_t b, size_t *c)
{
    if (b == 0) {
        *c = 0;
        return false;
    }

    if (a > SIZE_MAX / b) {
        return true;
    }

    *c = a * b;
    return false;
}
