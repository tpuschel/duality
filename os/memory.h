/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DUALITY_OS_MEMORY_H
#define DUALITY_OS_MEMORY_H

#include "uefi.h"

#include "../support/util.h"
#include "../support/overflow.h"

/**
 * Super simple (and slow) memory allocation implementation.
 *
 * Uses a linked list of contiguous memory blocks,
 * with each block containing slots of reference-counted memory regions.
 *
 * Currently, contiguous unused memory regions won't get coalesced.
 */

/**
 * A 'slot' is what gets allocated together with the actual data, as a preamble.
 *
 * All memory regions are reference counted.
 *
 * The last slot of each memory block has its is_end bit set.
 * This last slot does not indicate usable space; it's just used
 * to indicate the end of a block.
 */
struct mem_slot {
    size_t ref_cnt;
    struct {
        size_t is_end : 1;
        size_t size : (sizeof(size_t) * CHAR_BIT) - 1;
    };
};

/**
 * Since every allocation gets preceded and potentially succeeded by a mem_slot,
 * alignment requirements must be taken into account.
 *
 * To avoid having to recompute the required paddings everytime,
 * the macros below are meant to be used to initialize a static const
 * variable with the respective padding.
 *
 * Pass these two constant variables as the pre_ and post_padding to alloc/retain/release/change.
 */

#define MEM_SLOT_PRE_PADDING(type) \
    DY_COMPUTE_PADDING(sizeof(struct mem_slot), DY_ALIGNOF(type))

#define MEM_SLOT_POST_PADDING(type) \
    DY_COMPUTE_PADDING(sizeof(struct mem_slot) + MEM_SLOT_PRE_PADDING(type) + sizeof(type), DY_ALIGNOF(struct mem_slot))

/** The header of a memory block. */
struct mem_blk {
    struct mem_blk *next_block; // Can be NULL.
    struct mem_slot first_slot;
};

/** The global start of the blocks of memory. */
static struct mem_blk *mem_first_block = NULL;

static inline void mem_init(char *map, size_t map_size, size_t descriptor_size);

static inline void *mem_alloc(size_t size, size_t pre_padding, size_t post_padding);

static inline void *mem_retain(const void *ptr, size_t pre_padding, size_t post_padding);

static inline size_t mem_release(const void *ptr, size_t pre_padding, size_t post_padding);

static inline bool mem_change(void *ptr, size_t new_size, size_t pre_padding, size_t post_padding);

void mem_init(char *map, size_t map_size, size_t descriptor_size)
{
    struct mem_blk **prev_blocks_next_ptr = &mem_first_block;

    while (map_size >= descriptor_size) {
        struct efi_memory_descriptor *desc = map;

        size_t blk_size = desc->number_of_pages * 0x1000;

        /**
         * Memory of type EFI_BOOT_SERVICES_CODE/DATA should also be usable,
         * but QEMU hangs when writing to the end of one of these type of blocks :/.
         */
        if (desc->type == EFI_CONVENTIONAL_MEMORY) {
            struct mem_blk *blk = desc->physical_start; // Always suitably (4 KiB) aligned.

            blk->next_block = NULL;

            // Ok too, since the end is also always 4 KiB aligned.
            struct mem_slot *last_slot = (char *)blk + blk_size - sizeof *last_slot;
            last_slot->ref_cnt = 0xdeadbeef;
            last_slot->size = 0;
            last_slot->is_end = 1;

            blk->first_slot.ref_cnt = 0;
            blk->first_slot.is_end = 0;
            blk->first_slot.size = blk_size - sizeof *last_slot - sizeof *blk;

            *prev_blocks_next_ptr = blk;
            prev_blocks_next_ptr = &blk->next_block;
        }

        map += descriptor_size;
        map_size -= descriptor_size;
    }
}

void *mem_alloc(size_t size, size_t pre_padding, size_t post_padding)
{
    size_t actual_size = pre_padding + size + post_padding;

    for (struct mem_blk *blk = mem_first_block; blk != NULL; blk = blk->next_block) {
        for (struct mem_slot *slot = &blk->first_slot; !slot->is_end; slot = (char *)(slot + 1) + slot->size) {
            if (slot->ref_cnt != 0) {
                continue;
            }

            if (slot->size < actual_size) {
                continue;
            }

            size_t rest_size = slot->size - actual_size;

            if (rest_size == 0) {
                slot->ref_cnt = 1;
                slot->size = actual_size;
                slot->is_end = 0;
                return (char *)(slot + 1) + pre_padding;
            }

            if (rest_size < sizeof(struct mem_slot)) {
                continue;
            }

            struct mem_slot *new_slot = (char *)(slot + 1) + actual_size;
            new_slot->ref_cnt = 0;
            new_slot->size = rest_size;
            new_slot->is_end = 0;

            slot->ref_cnt = 1;
            slot->size = actual_size;
            slot->is_end = 0;

            return (char *)(slot + 1) + pre_padding;
        }
    }

    return NULL;
}

void *mem_retain(const void *ptr, size_t pre_padding, size_t post_padding)
{
    struct mem_slot *slot = (const char *)ptr - pre_padding - sizeof *slot;

    slot->ref_cnt++;

    return ptr;
}

size_t mem_release(const void *ptr, size_t pre_padding, size_t post_padding)
{
    struct mem_slot *slot = (const char *)ptr - pre_padding - sizeof *slot;

    return --slot->ref_cnt;
}

bool mem_change(void *ptr, size_t new_size, size_t pre_padding, size_t post_padding)
{
    struct mem_slot *slot = (char *)ptr - pre_padding - sizeof *slot;

    size_t old_size = slot->size - pre_padding - post_padding;

    if (old_size < new_size) {
        // Growing.

        struct mem_slot *next = (char *)slot + slot->size;
        if (next->ref_cnt != 0 || next->is_end) {
            return false;
        }

        size_t additional_size = new_size - old_size;

        size_t potential_size = sizeof *next + next->size;

        if (potential_size < additional_size) {
            // Not enough space.
            return false;
        }

        size_t leftover_space = potential_size - additional_size;

        if (leftover_space == 0) {
            // Exact fit.
            slot->size += additional_size;
            return true;
        }

        // Not an exact fit, so try to place another mem_slot.

        if (leftover_space < sizeof(struct mem_slot)) {
            return false;
        }

        slot->size += additional_size;

        struct mem_slot *new_next = (char *)slot + additional_size;
        new_next->ref_cnt = 0;
        new_next->is_end = 0;
        new_next->size = leftover_space - sizeof(struct mem_slot);

        return true;
    } else if (new_size < old_size) {
        // Shrinking.

        size_t diff = old_size - new_size;

        if (diff < sizeof(struct mem_slot)) {
            return false;
        }

        slot->size -= diff;

        struct mem_slot *new_slot = (char *)(slot + 1) + slot->size;
        new_slot->ref_cnt = 0;
        new_slot->is_end = 0;
        new_slot->size = diff - sizeof *new_slot;

        return true;
    }

    // Staying the same!
    return true;
}

#endif // DUALITY_OS_MEMORY_H
