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

#include "../syntax/utf8_to_ast.h"
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

void *dy_rc_alloc(size_t size, size_t alignment)
{
    return mem_alloc(size, alignment);
}

void *dy_rc_new(void *ptr, size_t size, size_t alignment)
{
    void *p = dy_rc_alloc(size, alignment);
    memcpy(p, ptr, size);
    return p;
}

void *dy_rc_retain(void *ptr, size_t alignment)
{
    return mem_retain(ptr, alignment);
}

size_t dy_rc_release(void *ptr, size_t alignment)
{
    return mem_release(ptr, alignment);
}

void *dy_rc_realloc(void *ptr, size_t new_size, size_t alignment)
{
    size_t pre_padding = MEM_SLOT_PRE_PADDING(alignment);

    if (mem_change(ptr, new_size, alignment)) {
        return ptr;
    }

    struct mem_slot *slot = (char *)ptr - pre_padding - sizeof *slot;
    size_t old_size = slot->size - pre_padding;

    void *p = dy_rc_alloc(new_size, alignment);

    memcpy(p, ptr, DY_MIN(old_size, new_size));

    mem_release(ptr, alignment);

    return p;
}
