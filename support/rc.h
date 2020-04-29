/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_RC_H
#define DY_RC_H

#include "alloc.h"
#include "util.h"

/**
 * Implements reference-counting allocation functions.
 */

/**
 * What actually gets allocated is 64-bit ref-count + padding + actual size of the object.
 */
struct dy_rc_slot {
    size_t ref_cnt;
    char padding_and_obj[];
};

/**
 * New/retain/release all take an offset parameter that depends
 * on the alignment and size of the object to be allocated.
 *
 * Use DY_RC_OFFSET_OF_TYPE(type) to precompute the required offset
 * for objects of a given type.
 */

#define DY_RC_COMPUTE_PADDING(size, alignment) \
    (size % alignment                          \
            ? alignment - (size % alignment)   \
            : 0)

#define DY_RC_PADDING(size, alignment)                                    \
    (alignment >= DY_ALIGNOF(struct dy_rc_slot)                           \
            ? DY_RC_COMPUTE_PADDING(sizeof(struct dy_rc_slot), alignment) \
            : DY_RC_COMPUTE_PADDING(size, DY_ALIGNOF(struct dy_rc_slot)))

#define DY_RC_OFFSET(size, alignment) \
    (DY_RC_PADDING(size, alignment) + sizeof(struct dy_rc_slot))

#define DY_RC_OFFSET_OF_TYPE(type) DY_RC_OFFSET(sizeof(type), DY_ALIGNOF(type))

/**
 * Copies 'size' bytes from 'ptr' to a reference-counted slot in the heap.
 * Returns a pointer to the object portion of that slot.
 */
static inline const void *dy_rc_new(const void *ptr, size_t size, size_t offset);

/**
 * Increments the reference count of the object pointed to by 'ptr'.
 * Returns 'ptr'.
 */
static inline const void *dy_rc_retain(const void *ptr, size_t offset);

/**
 * Decrements the reference count of the object pointed to by 'ptr'.
 * Frees the object if the new reference count is 0.
 * Returns the new reference count.
 */
static inline size_t dy_rc_release(const void *ptr, size_t offset);

const void *dy_rc_new(const void *ptr, size_t size, size_t offset)
{
    struct dy_rc_slot *slot = dy_calloc(1, offset + size);

    ++slot->ref_cnt;

    void *p = (char *)slot + offset;

    memcpy(p, ptr, size);

    return p;
}

const void *dy_rc_retain(const void *ptr, size_t offset)
{
    struct dy_rc_slot *slot = (void *)((char *)ptr - offset);

    ++slot->ref_cnt;

    return ptr;
}

size_t dy_rc_release(const void *ptr, size_t offset)
{
    struct dy_rc_slot *slot = (void *)((char *)ptr - offset);

    size_t new_ref_cnt = --slot->ref_cnt;

    if (new_ref_cnt == 0) {
        dy_free(slot);
    }

    return new_ref_cnt;
}

#endif // DY_RC_H
