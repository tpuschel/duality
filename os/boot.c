/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "uefi.h"
#include "memory.h"
#include "duality.h"
#include "freestanding.h"

/**
 * Entry point into the OS from UEFI.
 */
EFIAPI size_t boot(void *image_handle, struct efi_sys_tab *sys_tab)
{
    static struct efi_guid gop_guid = { 0x9042a9de, 0x23dc, 0x4a38, { 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a } };

    struct efi_gfx_out_prot *gop;
    size_t status = sys_tab->boot_services->locate_protocol(&gop_guid, NULL, (void *)&gop);
    if (status != EFI_SUCCESS) {
        size_t size;
        uint16_t *s = efi_string(image_handle, sys_tab->boot_services, STR_LIT("Unable to locate GOP."), &size);
        sys_tab->boot_services->exit(image_handle, status, size, s);
    }

    struct efi_gfx_out_mode_info mode_info;
    size_t frame_buffer_size;
    uint32_t *frame_buffer = set_gfx_mode(image_handle, sys_tab->boot_services, gop, &frame_buffer_size, &mode_info);

    static const struct efi_guid acpi_guid = { 0x8868e871, 0xe4f1, 0x11d3, { 0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81 } };

    void *acpi_tab = NULL;
    for (size_t i = 0; i < sys_tab->number_of_table_entries; ++i) {
        struct efi_config_tab tab = sys_tab->configuration_table[i];
        if (memcmp(&tab.vendor_guid, &acpi_guid, sizeof(struct efi_guid)) == 0) {
            acpi_tab = tab.vendor_table;
            break;
        }
    }

    if (!acpi_tab) {
        size_t size;
        uint16_t *s = efi_string(image_handle, sys_tab->boot_services, STR_LIT("Unable to locate ACPI table."), &size);
        sys_tab->boot_services->exit(image_handle, EFI_NOT_FOUND, size, s);
    }

    char *memory_map = efi_alloc(image_handle, sys_tab->boot_services, 0);
    size_t memory_map_size;
    size_t memory_descriptor_size;
    size_t memory_map_key = get_memory_map(image_handle, sys_tab->boot_services, memory_map, &memory_map_size, &memory_descriptor_size);

    status = sys_tab->boot_services->exit_boot_services(image_handle, memory_map_key);
    if (status != EFI_SUCCESS) {
        size_t size;
        uint16_t *s = efi_string(image_handle, sys_tab->boot_services, STR_LIT("Unable to exit boot services."), &size);
        sys_tab->boot_services->exit(image_handle, status, size, s);
    }

    mem_init(memory_map, memory_map_size, memory_descriptor_size);

    // If the whole screen's white after booting, we know it's working :).
    for (size_t i = 0; i < frame_buffer_size / 4; ++i) {
        frame_buffer[i] = 0x00ffffff;
    }

    __asm__ volatile("hlt");

    return EFI_SUCCESS;
}
