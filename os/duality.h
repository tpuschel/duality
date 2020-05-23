/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "memory.h"
#include "freestanding.h"

/**
 * Defines functions and macros needed for Duality.
 */

#define DY_FREESTANDING

#define assert(x)    \
    do {             \
        if (!(x)) {  \
            abort(); \
        }            \
    } while (0)

#define DY_RC_PRE_PADDING MEM_SLOT_PRE_PADDING
#define DY_RC_POST_PADDING MEM_SLOT_POST_PADDING

DY_NORETURN void abort(void);
DY_NORETURN void dy_bail(const char *s);

#include "../syntax/parser.h"
#include "../syntax/ast_to_core.h"

void abort(void)
{
    __asm__ volatile("hlt");
    __builtin_unreachable();
}

void dy_bail(const char *s)
{
    abort();
}

void *dy_rc_alloc(size_t size, size_t pre_padding, size_t post_padding)
{
    return mem_alloc(size, pre_padding, post_padding);
}

const void *dy_rc_new(const void *ptr, size_t size, size_t pre_padding, size_t post_padding)
{
    void *p = dy_rc_alloc(size, pre_padding, post_padding);
    memcpy(p, ptr, size);
    return p;
}

const void *dy_rc_retain(const void *ptr, size_t pre_padding, size_t post_padding)
{
    return mem_retain(ptr, pre_padding, post_padding);
}

size_t dy_rc_release(const void *ptr, size_t pre_padding, size_t post_padding)
{
    return mem_release(ptr, pre_padding, post_padding);
}

void *dy_rc_realloc(void *ptr, size_t new_size, size_t pre_padding, size_t post_padding)
{
    if (mem_change(ptr, new_size, pre_padding, post_padding)) {
        return ptr;
    }

    struct mem_slot *slot = (char *)ptr - pre_padding - sizeof *slot;
    size_t old_size = slot->size - pre_padding - post_padding;

    void *p = dy_rc_alloc(new_size, pre_padding, post_padding);

    memcpy(p, ptr, DY_MIN(old_size, new_size));

    mem_release(ptr, pre_padding, post_padding);

    return p;
}
