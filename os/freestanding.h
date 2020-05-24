/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>

/** Functions needed by gcc/clang even in freestanding mode. */

void *memcpy(void *restrict dst, const void *restrict src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memset(void *b, int c, size_t len);
int memcmp(const void *s1, const void *s2, size_t n);

void *memcpy(void *dst, const void *src, size_t n)
{
    char *p = dst;
    const char *p2 = src;

    for (size_t i = 0; i < n; ++i) {
        p[i] = p2[i];
    }

    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n); // TODO: Actually implement this :D
}

void *memset(void *b, int c, size_t len)
{
    unsigned char *p = b;

    for (size_t i = 0; i < len; ++i) {
        p[i] = (unsigned char)c;
    }

    return b;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = s1, *p2 = s2;

    for (size_t i = 0; i < n; ++i) {
        int diff = (int)p1[i] - (int)p2[i];
        if (diff != 0) {
            return diff;
        }
    }

    return 0;
}
