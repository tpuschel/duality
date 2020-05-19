/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * We only define as much UEFI stuff as we need.
 *
 * To avoid having to type the full signature of function pointers
 * that are not used, void * is used instead.
 * Void * not guaranteed to be the same size as a function pointer,
 * but it effectively is on anything UEFI supports (AFAIK).
 */

#define EFIAPI __attribute((ms_abi))

static const size_t EFI_SUCCESS = 0;
static const size_t EFI_BUFFER_TOO_SMALL = 5;
static const size_t EFI_NOT_FOUND = 14;

struct efi_guid {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t data4[8];
};

struct efi_tab_hdr {
    uint64_t signature;
    uint32_t revision;
    uint32_t header_size;
    uint32_t crc32;
    uint32_t reserved;
};

struct efi_sys_tab {
    struct efi_tab_hdr hdr;
    uint16_t *firmware_vendor;
    uint32_t firmware_revision;
    void *console_in_handle;
    void *con_in;
    void *console_out_handle;
    void *con_out;
    void *standard_error_handle;
    void *std_err;
    void *runtime_services;
    struct efi_boot_services *boot_services;
    size_t number_of_table_entries;
    void *configuration_table;
};

struct efi_memory_descriptor {
    uint32_t type;
    void *physical_start;
    void *virtual_start;
    uint64_t number_of_pages;
    uint64_t attribute;
};

enum efi_memory_type {
    EFI_RESERVED,
    EFI_LOADER_CODE,
    EFI_LOADER_DATA
};

struct efi_boot_services {
    struct efi_tab_hdr hdr;

    void *raise_tpl;
    void *restore_tpl;

    void *allocate_pages;
    void *free_pages;
    size_t(EFIAPI *get_memory_map)(size_t *memory_map_size, struct efi_memory_descriptor *memory_map, size_t *map_key, size_t *descriptor_size, uint32_t *descriptor_version);
    size_t(EFIAPI *allocate_pool)(enum efi_memory_type pool_type, size_t size, void **buffer);
    size_t(EFIAPI *free_pool)(void *buffer);

    void *create_event;
    void *set_timer;
    void *wait_for_event;
    void *signal_event;
    void *close_event;
    void *check_event;

    void *install_protocol_interface;
    void *reinstall_protocol_interface;
    void *uninstall_protocol_interface;
    void *handle_protocol;
    void *reserved;
    void *register_protocol_notifiy;
    void *locate_handle;
    void *locate_device_path;
    void *install_configuration_table;

    void *image_load;
    void *image_start;
    size_t(EFIAPI *exit)(void *image_handle, size_t exit_status, size_t exit_data_size, uint16_t *exit_data);
    void *image_unload;
    size_t(EFIAPI *exit_boot_services)(void *image_handle, size_t map_key);

    void *get_next_monotonic_count;
    void *stall;
    void *set_watchdog_timer;

    void *connect_controller;
    void *disconnect_controller;

    void *open_protocol;
    void *close_protocol;
    void *open_protocol_information;

    void *protocols_per_handle;
    void *locate_handle_buffer;
    size_t(EFIAPI *locate_protocol)(struct efi_guid *protocol, void *registration, void **interface);
    void *install_multiple_protocol_interfaces;
    void *uninstall_multiple_protocol_interfaces;

    void *calculate_crc32;

    void *copy_mem;
    void *set_mem;
    void *create_event_ex;
};

enum efi_gfx_pix_fmt {
    EFI_PIXEL_RGBR_8BIT,
    EFI_PIXEL_BGRR_8BIT
};

struct efi_gfx_pix_bitmask {
    uint32_t red;
    uint32_t green;
    uint32_t blue;
    uint32_t reserved;
};

struct efi_gfx_out_mode_info {
    uint32_t version;
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    enum efi_gfx_pix_fmt pix_fmt;
    struct efi_gfx_pix_bitmask pix_info;
    uint32_t pixels_per_scanline;
};

struct efi_gfx_mode {
    uint32_t max_mode;
    uint32_t mode;
    struct efi_gfx_out_mode_info *info;
    size_t size_of_info;
    void *frame_buffer_base;
    size_t frame_buffer_size;
};

struct efi_gfx_out_prot {
    size_t(EFIAPI *query_mode)(struct efi_gfx_out_prot *this, uint32_t mode_number, size_t *size_of_info, struct efi_gfx_out_mode_info **info);

    size_t(EFIAPI *set_mode)(struct efi_gfx_out_prot *this, uint32_t mode_number);

    void *blt;

    struct efi_gfx_mode *mode;
};

/**
 * Both gcc and clang emit calls to memcpy even in freestanding mode.
 */
void memcpy(void *restrict dst, void *restrict src, size_t size);

/**
 * Malloc/Free using UEFI's pool allocation functions.
 * Everything's allocated from LoaderData type memory.
 */
static void *efi_alloc(void *image, struct efi_boot_services *boot_services, size_t size);
static void efi_free(void *image, struct efi_boot_services *boot_services, void *ptr);

/**
 * Utility function to get the memory map.
 * Needed because we need to allocate space to whold the map,
 * and that allocation changes the map itself.
 */
static size_t get_memory_map(void *image, struct efi_boot_services *boot_services, struct efi_memory_descriptor *map, size_t *map_size, size_t *descriptor_size);

/**
 * Set's the highest resolution mode that supports 32-bit BGR color.
 * Returns frame buffer and mode info.
 */
static uint32_t *set_gfx_mode(void *image, struct efi_boot_services *boot_services, struct efi_gfx_out_prot *gop, size_t *frame_buffer_size, struct efi_gfx_out_mode_info *mode_info);

/**
 * Entry point into the OS from UEFI.
 */
EFIAPI size_t boot(void *image_handle, struct efi_sys_tab *sys_tab)
{
    struct efi_guid gop_guid = { 0x9042a9de, 0x23dc, 0x4a38, { 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a } };

    struct efi_gfx_out_prot *gop;
    size_t status = sys_tab->boot_services->locate_protocol(&gop_guid, NULL, (void *)&gop);
    if (status != EFI_SUCCESS) {
        return status;
    }

    struct efi_gfx_out_mode_info mode_info;
    size_t frame_buffer_size;
    uint32_t *frame_buffer = set_gfx_mode(image_handle, sys_tab->boot_services, gop, &frame_buffer_size, &mode_info);

    struct efi_memory_descriptor *memory_map = efi_alloc(image_handle, sys_tab->boot_services, 0);
    size_t memory_map_size;
    size_t memory_descriptor_size;
    size_t memory_map_key = get_memory_map(image_handle, sys_tab->boot_services, memory_map, &memory_map_size, &memory_descriptor_size);

    status = sys_tab->boot_services->exit_boot_services(image_handle, memory_map_key);
    if (status != EFI_SUCCESS) {
        return status;
    }

    for (size_t i = 0; i < frame_buffer_size / 4; ++i) {
        frame_buffer[i] = 0x00ffffff;
    }

    __asm__("hlt");

    return EFI_SUCCESS;
}

size_t get_memory_map(void *image, struct efi_boot_services *boot_services, struct efi_memory_descriptor *map, size_t *map_size, size_t *descriptor_size)
{
    uint32_t descriptor_version;
    size_t map_key;
    for (;;) {
        size_t status = boot_services->get_memory_map(map_size, map, &map_key, descriptor_size, &descriptor_version);
        if (status == EFI_BUFFER_TOO_SMALL) {
            efi_free(image, boot_services, map);

            *map_size += 64; // Little bit extra to account for the bookkeeping of the next allocation.

            map = efi_alloc(image, boot_services, *map_size);
        }

        if (status != EFI_SUCCESS) {
            boot_services->exit(image, status, 0, NULL);
        }

        return map_key;
    }
}

uint32_t *set_gfx_mode(void *image, struct efi_boot_services *boot_services, struct efi_gfx_out_prot *gop, size_t *frame_buffer_size, struct efi_gfx_out_mode_info *mode_info)
{
    struct efi_gfx_out_mode_info *mode_info_ptr = NULL;
    size_t mode_size;
    uint32_t resolution = 0;
    uint32_t mode = 0;

    for (uint32_t i = 0; i < gop->mode->max_mode; ++i) {
        struct efi_gfx_out_mode_info *new_mode_info_ptr = NULL;
        size_t status = gop->query_mode(gop, i, &mode_size, &new_mode_info_ptr);
        if (status != EFI_SUCCESS) {
            boot_services->exit(image, status, 0, NULL);
        }

        if (new_mode_info_ptr->pix_fmt != EFI_PIXEL_BGRR_8BIT) {
            continue;
        }

        uint32_t new_resolution = new_mode_info_ptr->horizontal_resolution * new_mode_info_ptr->vertical_resolution;

        if (mode_info_ptr == NULL || new_resolution > resolution) {
            mode_info_ptr = new_mode_info_ptr;
            resolution = new_resolution;
            mode = i;
        }
    }

    if (mode_info_ptr == NULL) {
        boot_services->exit(image, EFI_NOT_FOUND, 0, NULL);
    }

    size_t status = gop->set_mode(gop, mode);
    if (status != EFI_SUCCESS) {
        boot_services->exit(image, status, 0, NULL);
    }

    *mode_info = *gop->mode->info;
    *frame_buffer_size = gop->mode->frame_buffer_size;
    return gop->mode->frame_buffer_base;
}

void *efi_alloc(void *image, struct efi_boot_services *boot_services, size_t size)
{
    void *ptr;
    size_t status = boot_services->allocate_pool(EFI_LOADER_DATA, size, &ptr);
    if (status != EFI_SUCCESS) {
        boot_services->exit(image, status, 0, NULL);
    }
    return ptr;
}

void efi_free(void *image, struct efi_boot_services *boot_services, void *ptr)
{
    size_t status = boot_services->free_pool(ptr);
    if (status != EFI_SUCCESS) {
        boot_services->exit(image, status, 0, NULL);
    }
}

void memcpy(void *restrict dst, void *restrict src, size_t size)
{
    char *p = dst, *p2 = src;

    for (size_t i = 0; i < size; ++i) {
        p[i] = p2[i];
    }
}
