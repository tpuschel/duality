/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

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

#define DY_MAX(x, y) ((x) < (y) ? (y) : (x))
#define DY_MIN(x, y) ((x) < (y) ? (x) : (y))

#define DY_COMPUTE_PADDING(size, alignment)        \
    ((size) % (alignment)                          \
            ? (alignment) - ((size) % (alignment)) \
            : 0)
