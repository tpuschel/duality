/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DUALITY_OS_MEMORY_H
#define DUALITY_OS_MEMORY_H

#include "uefi.h"

/**
 * Super simple (and slow) memory allocation implementation.
 *
 * Uses a doubly-linked list of alternating free/used nodes.
 * Coalesces neighbors on mem_free().
 *
 * Allocating is O(n) where n = number of free/used nodes.
 * Freeing is O(1).
 */

struct mem_node {
    size_t free_space;

    struct mem_node *prev;
    struct mem_node *next;

    char data[];
};

static struct mem_node *mem_head = NULL;

static void mem_init(char *map, size_t map_size, size_t descriptor_size);

static void *mem_alloc(size_t size);

static void mem_free(void *ptr, size_t size);

void mem_init(char *map, size_t map_size, size_t descriptor_size)
{
    struct mem_node **prev_node = &mem_head;

    for (;;) {
        if (map_size < descriptor_size) {
            break;
        }

        struct efi_memory_descriptor *desc = map;

        if (desc->type == EFI_BOOT_SERVICES_CODE || desc->type == EFI_BOOT_SERVICES_DATA || desc->type == EFI_CONVENTIONAL_MEMORY) {
            struct mem_node *node = desc->physical_start;
            node->free_space = (desc->number_of_pages * 0x1000) - sizeof *node;
            node->prev = *prev_node;
            node->next = NULL;

            *prev_node = node;
            prev_node = &node->next;
        }

        map += descriptor_size;
        map_size -= descriptor_size;
    }
}

void *mem_alloc(size_t size)
{
    struct mem_node *node;
    for (node = mem_head; node != NULL; node = node->next) {
        if (node->free_space - sizeof *node >= size) {
            break;
        }
    }

    if (node == NULL) {
        return NULL;
    }

    struct mem_node *new_node = node->data + size;
    new_node->free_space = node->free_space - size;
    new_node->prev = node;
    new_node->next = node->next;

    node->next->prev = new_node;
    node->next = new_node;
    node->free_space = 0;

    return node->data;
}

void mem_free(void *ptr, size_t size)
{
    struct mem_node *node = (char *)ptr - sizeof *node;

    node->free_space = size;

    // Coalesce with the left neighbor if free.
    if ((char *)node->prev->data + node->prev->free_space == (char *)node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->prev->free_space += sizeof *node + node->free_space;

        node = node->prev;
    }

    // Coalesce with the right neighbor if free.
    if (node->data + node->free_space == (char *)node->next && node->next->free_space != 0) {
        node->next = node->next->next;
        node->next->prev = node;
        node->free_space += sizeof *node->next + node->next->free_space;
    }
}

#endif // DUALITY_OS_MEMORY_H
