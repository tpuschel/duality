/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

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

#define STR_LIT(x) x, sizeof(x)

#define HIGH_BIT(type) (~(~(type)0 >> 1))

#define EFI_ERROR(x) (HIGH_BIT(size_t) + x)

static const size_t EFI_SUCCESS = 0;
static const size_t EFI_BUFFER_TOO_SMALL = EFI_ERROR(5);
static const size_t EFI_NOT_FOUND = EFI_ERROR(14);

static const uint64_t EFI_MEMORY_UC = 0x1;
static const uint64_t EFI_MEMORY_WC = 0x2;
static const uint64_t EFI_MEMORY_WT = 0x4;
static const uint64_t EFI_MEMORY_WB = 0x8;
static const uint64_t EFI_MEMORY_UCE = 0x10;
static const uint64_t EFI_MEMORY_WP = 0x1000;
static const uint64_t EFI_MEMORY_RP = 0x2000;
static const uint64_t EFI_MEMORY_XP = 0x4000;
static const uint64_t EFI_MEMORY_NV = 0x8000;
static const uint64_t EFI_MEMORY_MORE_RELIABLE = 0x10000;
static const uint64_t EFI_MEMORY_RO = 0x20000;
static const uint64_t EFI_MEMORY_SP = 0x40000;
static const uint64_t EFI_MEMORY_CPU_CRYPTO = 0x80000;
static const uint64_t EFI_MEMORY_RUNTIME = HIGH_BIT(uint64_t);

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
    struct efi_config_tab *configuration_table;
};

struct efi_config_tab {
    struct efi_guid vendor_guid;
    void *vendor_table;
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
    EFI_LOADER_DATA,
    EFI_BOOT_SERVICES_CODE,
    EFI_BOOT_SERVICES_DATA,
    EFI_RUNTIME_SERVICES_CODE,
    EFU_RUNTIME_SERVICES_DATA,
    EFI_CONVENTIONAL_MEMORY,
    EFI_UNUSABLE_MEMORY,
    EFI_ACPI_RECLAIM_MEMORY,
    EFI_ACPI_MEMORY_NVS,
    EFI_MEMORY_MAPPED_IO,
    EFI_MEMORY_MAPPED_IO_PORT_SPACE,
    EFI_PAL_CODE,
    EFI_PERSISTENT_MEMORY
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
    EFI_PIXEL_RGBReserved_8BIT,
    EFI_PIXEL_BGRReserved_8BIT
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
 * Malloc/Free using UEFI's pool allocation functions.
 * Everything's allocated from LoaderData type memory.
 */
static inline void *efi_alloc(void *image, struct efi_boot_services *boot_services, size_t size);
static inline void efi_free(void *image, struct efi_boot_services *boot_services, void *ptr);

static inline uint16_t *efi_string(void *image, struct efi_boot_services *boot_services, const char *s, size_t byte_size, size_t *utf16_size);

/**
 * Utility function to get the memory map.
 * Needed because we need to allocate space to hold the map,
 * and that allocation changes the map itself.
 */
static inline size_t get_memory_map(void *image, struct efi_boot_services *boot_services, char *map, size_t *map_size, size_t *descriptor_size);

/**
 * Set's the highest resolution mode that supports 32-bit BGR color.
 * Returns frame buffer and mode info.
 */
static inline uint32_t *set_gfx_mode(void *image, struct efi_boot_services *boot_services, struct efi_gfx_out_prot *gop, size_t *frame_buffer_size, struct efi_gfx_out_mode_info *mode_info);

size_t get_memory_map(void *image, struct efi_boot_services *boot_services, char *map, size_t *map_size, size_t *descriptor_size)
{
    uint32_t descriptor_version;
    size_t map_key;
    for (;;) {
        size_t status = boot_services->get_memory_map(map_size, map, &map_key, descriptor_size, &descriptor_version);
        if (status == EFI_BUFFER_TOO_SMALL) {
            efi_free(image, boot_services, map);

            *map_size += 64; // Little bit extra to account for the bookkeeping of the next allocation.

            map = efi_alloc(image, boot_services, *map_size);

            continue;
        }

        if (status != EFI_SUCCESS) {
            size_t size;
            uint16_t *s = efi_string(image, boot_services, STR_LIT("Unable to get memory map."), &size);
            boot_services->exit(image, status, size, s);
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
            size_t size;
            uint16_t *s = efi_string(image, boot_services, STR_LIT("Unable to query GOP mode."), &size);
            boot_services->exit(image, status, size, s);
        }

        if (new_mode_info_ptr->pix_fmt != EFI_PIXEL_BGRReserved_8BIT) {
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
        size_t size;
        uint16_t *s = efi_string(image, boot_services, STR_LIT("No suitable GOP mode found."), &size);
        boot_services->exit(image, EFI_NOT_FOUND, size, s);
    }

    size_t status = gop->set_mode(gop, mode);
    if (status != EFI_SUCCESS) {
        size_t size;
        uint16_t *s = efi_string(image, boot_services, STR_LIT("Unable to set GOP mode."), &size);
        boot_services->exit(image, status, size, s);
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
        size_t s_size;
        uint16_t *s = efi_string(image, boot_services, STR_LIT("Unable to allocate memory."), &s_size);
        boot_services->exit(image, status, s_size, s);
    }
    return ptr;
}

void efi_free(void *image, struct efi_boot_services *boot_services, void *ptr)
{
    size_t status = boot_services->free_pool(ptr);
    if (status != EFI_SUCCESS) {
        size_t size;
        uint16_t *s = efi_string(image, boot_services, STR_LIT("Unable to free memory."), &size);
        boot_services->exit(image, status, size, s);
    }
}

uint16_t *efi_string(void *image, struct efi_boot_services *boot_services, const char *s, size_t byte_size, size_t *utf16_size)
{
    // This will only work if 's' isn't multibyte encoded.

    size_t new_size = byte_size * 2;

    uint16_t *ret_s = efi_alloc(image, boot_services, new_size);

    for (size_t i = 0; i < byte_size; ++i) {
        ret_s[i] = (uint16_t)s[i];
    }

    *utf16_size = new_size;

    return ret_s;
}
