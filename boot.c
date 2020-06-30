/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "os/uefi.h"
#include "os/memory.h"
#include "os/duality.h"
#include "os/freestanding.h"
#include "os/cpu.h"
#include "os/acpi.h"

static inline void cpu_features(bool *x2apic_supported, uint32_t *initial_apic_id);

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

    struct acpi_rsdp *rsdp = NULL;
    for (size_t i = 0; i < sys_tab->number_of_table_entries; ++i) {
        struct efi_config_tab tab = sys_tab->configuration_table[i];
        if (memcmp(&tab.vendor_guid, &acpi_guid, sizeof(struct efi_guid)) == 0) {
            rsdp = tab.vendor_table;
            break;
        }
    }

    if (!rsdp) {
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

    bool x2apic_supported;
    uint32_t apic_id;
    cpu_features(&x2apic_supported, &apic_id);

    load_gdt();

    load_idt();

    __asm__ volatile goto("jmp %l0" :: ::ap_end);

ap_start:;
    __asm__ volatile(
        "mov $1337, %%rax;"
        "hlt;"
        :
        :
        : "rax");
ap_end:;

    size_t ap_code_size = (size_t)((char *)&&ap_end - (char *)&&ap_start);

    // 4KiB aligned sub-1MiB address to put the above asm routine into.
    // Initially set to 1MiB to detect failure (since 0/NULL is a valid address).
    void *ap_code_space = (void *)0x100000;
    mem_init(memory_map, memory_map_size, memory_descriptor_size, &ap_code_space, ap_code_size);

    if ((uint64_t)ap_code_space == 0x100000) {
        // Nowhere to put the AP init code :/
        size_t size;
        uint16_t *s = efi_string(image_handle, sys_tab->boot_services, STR_LIT("Unable to find sub-1MiB space to put the AP init code."), &size);
        sys_tab->boot_services->exit(image_handle, EFI_NOT_FOUND, size, s);
    }

    memcpy(ap_code_space, &&ap_start, ap_code_size);

    struct acpi_xsdt *xsdt = (void *)rsdp->xsdt_addr;

    if (xsdt->hdr.length < sizeof *xsdt) {
        size_t size;
        uint16_t *s = efi_string(image_handle, sys_tab->boot_services, STR_LIT("Invalid ACPI XSDT header length."), &size);
        sys_tab->boot_services->exit(image_handle, EFI_NOT_FOUND, size, s);
    }

    struct acpi_madt *madt = NULL;
    for (uint32_t i = 0; i < (xsdt->hdr.length - sizeof *xsdt) / sizeof(uint64_t); ++i) {
        struct acpi_desc_hdr *hdr = (void *)xsdt->entries[i];
        if (memcmp(hdr->signature, "APIC", 4) == 0) {
            madt = (struct acpi_madt *)hdr;
            break;
        }
    }

    if (!madt) {
        size_t size;
        uint16_t *s = efi_string(image_handle, sys_tab->boot_services, STR_LIT("Unable to find ACPI MADT."), &size);
        sys_tab->boot_services->exit(image_handle, EFI_NOT_FOUND, size, s);
    }

    if (madt->hdr.length < sizeof *madt) {
        size_t size;
        uint16_t *s = efi_string(image_handle, sys_tab->boot_services, STR_LIT("Invalid ACPI MADT header length."), &size);
        sys_tab->boot_services->exit(image_handle, EFI_NOT_FOUND, size, s);
    }

    uint8_t vector = (uint8_t)(((uint64_t)ap_code_space & 0xff000) >> 6);

    if (x2apic_supported) {
        enable_x2apic();
        // Read the APIC ID from the register in case it's not the same as the initial one.
        apic_id = (uint32_t)rdmsr(0x802);
    }

    size_t num_cpus = 0;
    uint8_t *p = madt->intr_ctrls;
    if (x2apic_supported) {
        for (uint32_t len = madt->hdr.length - sizeof *madt; len != 0;) {
            if (p[0] != acpi_x2apic_type) {
                len -= p[1];
                p += p[1];
                continue;
            }

            struct acpi_x2apic *apic = p;

            if (!apic->enabled && !apic->online_capable) {
                len -= apic->length;
                p += apic->length;
                continue;
            }

            num_cpus++;

            if (apic->x2apic_id != apic_id) {
                send_init_ipi_x2apic(apic->x2apic_id);

                send_startup_ipi_x2apic(apic->x2apic_id, vector);
            }

            len -= apic->length;
            p += apic->length;
        }
    } else {
        for (uint32_t len = madt->hdr.length - sizeof *madt; len != 0;) {
            if (p[0] != acpi_apic_type) {
                len -= p[1];
                p += p[1];
                continue;
            }

            struct acpi_apic *apic = p;

            if (!apic->enabled && !apic->online_capable) {
                len -= apic->length;
                p += apic->length;
                continue;
            }

            num_cpus++;

            if (apic->apic_id != apic_id) {
                send_init_ipi_xapic(madt->local_intr_ctrl_addr, apic->apic_id);

                send_startup_ipi_xapic(madt->local_intr_ctrl_addr, apic->apic_id, vector);
            }

            len -= apic->length;
            p += apic->length;
        }
    }

    // If the whole screen's white after booting, we know it's working :).
    for (size_t i = 0; i < frame_buffer_size / 4; ++i) {
        frame_buffer[i] = 0x00ffffff;
    }

    for (size_t i = 0; i < num_cpus * 2; i += 2) {
        frame_buffer[i] = 0x00000000;
    }

    __asm__ volatile("hlt");

    return EFI_SUCCESS;
}

void cpu_features(bool *x2apic_supported, uint32_t *initial_apic_id)
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(0, &eax, &ebx, &ecx, &edx);

    uint32_t max_eax = eax;

    if (max_eax >= 0x1f) {
        // Leaf 0x1f supported, prefer to 0x0b
        cpuid(0x1f, &eax, &ebx, &ecx, &edx);

        // We just assume x2apic is supported without checking the feature bit
        // if the leaf containing the 32-bit x2apic id is supported.
        *x2apic_supported = true;
        *initial_apic_id = edx;
        return;
    }

    if (max_eax >= 0xb) {
        cpuid(0xb, &eax, &ebx, &ecx, &edx);

        *x2apic_supported = true; // Same reasoning as above.
        *initial_apic_id = edx;
        return;
    }

    cpuid(1, &eax, &ebx, &ecx, &edx);

    *x2apic_supported = (ecx >> 21) & 1;
    *initial_apic_id = (ebx >> 24) & 0xf;
}
