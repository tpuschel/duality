/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_UTIL_H
#define DY_UTIL_H

/**
 * Some utility macros.
 */

#ifdef _MSC_VER
#    define DY_NORETURN __declspec(noreturn)
#    define DY_ALIGNOF __alignof
#else
#    define DY_NORETURN __attribute__((noreturn))
#    define DY_ALIGNOF __alignof__
#endif

#define DY_MAX(x, y) (x) < (y) ? (y) : (x)

#define DY_MAX_ALIGNMENT DY_MAX(DY_ALIGNOF(intmax_t), DY_MAX(DY_ALIGNOF(void *), DY_ALIGNOF(long double)))

#endif // DY_UTIL_H
