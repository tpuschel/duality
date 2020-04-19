/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_RC_H
#define DY_RC_H

#include "alloc.h"
#include "util.h"

struct dy_rc_slot {
    size_t ref_cnt;
    char obj[];
};

#define DY_RC_COMPUTE_PADDING(size, alignment) \
    (size % alignment                          \
            ? alignment - (size % alignment)   \
            : 0)

#define DY_RC_PADDING(size, alignment)                                    \
    (alignment >= DY_ALIGNOF(struct dy_rc_slot)                             \
            ? DY_RC_COMPUTE_PADDING(sizeof(struct dy_rc_slot), alignment) \
            : DY_RC_COMPUTE_PADDING(size, DY_ALIGNOF(struct dy_rc_slot)))

#define DY_RC_OFFSET(size, alignment) \
    (DY_RC_PADDING(size, alignment) + sizeof(struct dy_rc_slot))

#define DY_RC_OFFSET_OF_TYPE(type) DY_RC_OFFSET(sizeof(type), DY_ALIGNOF(type))

static inline const void *dy_rc_new(const void *ptr, size_t size, size_t offset);
static inline const void *dy_rc_retain(const void *ptr, size_t offset);
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
