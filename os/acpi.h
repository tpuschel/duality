/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

struct acpi_rsdp {
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

struct acpi_desc_hdr {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemid[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct acpi_xsdt {
    struct acpi_desc_hdr hdr;
    uint64_t entries[];
} __attribute__((packed));

struct acpi_madt {
    struct acpi_desc_hdr hdr;
    uint32_t local_intr_ctrl_addr;
    uint32_t flags;
    uint8_t intr_ctrls[];
} __attribute__((packed));

static const uint8_t acpi_apic_type = 0;
static const uint8_t acpi_x2apic_type = 9;

struct acpi_apic {
    uint8_t type;
    uint8_t length;
    uint8_t acpi_processor_id;
    uint8_t apic_id;
    struct {
        uint32_t enabled : 1;
        uint32_t online_capable : 1;
        uint32_t res2 : 30;
    };
} __attribute__((packed));

struct acpi_x2apic {
    uint8_t type;
    uint8_t length;
    uint8_t res[2];
    uint32_t x2apic_id;
    struct {
        uint32_t enabled : 1;
        uint32_t online_capable : 1;
        uint32_t res2 : 30;
    };
    uint32_t acpi_proc_uid;
} __attribute__((packed));
