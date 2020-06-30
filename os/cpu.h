/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

/**
 * Defines CPU-related stuff.
 */

struct code_seg {
    uint64_t ign1 : 42;
    uint64_t c : 1;
    uint64_t one : 1;
    uint64_t one2 : 1;
    uint64_t dpl : 2;
    uint64_t p : 1;
    uint64_t ign2 : 5;
    uint64_t l : 1;
    uint64_t d : 1;
    uint64_t ign3 : 9;
};

struct data_seg {
    uint64_t ign : 43;
    uint64_t zero : 1;
    uint64_t one : 1;
    uint64_t ign2 : 2;
    uint64_t p : 1;
    uint64_t ign3 : 16;
};

union gdt_elem {
    struct code_seg code;
    struct data_seg data;
};

struct desc_reg {
    uint16_t limit;
    uint64_t base_addr;
} __attribute__((packed));

struct seg_sel {
    uint16_t rpl : 2;
    uint16_t ti : 1;
    uint16_t si : 13;
};

struct idt_gate {
    uint64_t target_off1 : 16;
    uint64_t target_sel : 16;
    uint64_t ist : 3;
    uint64_t ign1 : 5;
    uint64_t type : 4;
    uint64_t zero : 1;
    uint64_t dpl : 2;
    uint64_t p : 1;
    uint64_t target_off2 : 48;
    uint64_t ign2 : 32;
};

enum idt_gate_type {
    IDT_GATE_INTERRUPT = 0,
    IDT_GATE_TRAP = 1
};

struct icr_x2apic {
    uint64_t vector : 8;
    uint64_t del_mode : 3;
    uint64_t dest_mode : 1;
    uint64_t res1 : 2;
    uint64_t level : 1;
    uint64_t trigger_mode : 1;
    uint64_t res2 : 2;
    uint64_t dest_shorthand : 2;
    uint64_t res3 : 12;
    uint64_t dest : 32;
};

struct icr_xapic_low {
    uint32_t vector : 8;
    uint32_t del_mode : 3;
    uint32_t dest_mode : 1;
    uint32_t del_status : 1;
    uint32_t res1 : 1;
    uint32_t level : 1;
    uint32_t trigger_mode : 1;
    uint32_t res2 : 2;
    uint32_t dest_shorthand : 2;
    uint32_t res3 : 12;
};

struct icr_xapic_high {
    uint32_t res : 24;
    uint32_t dest : 8;
};

static inline void load_gdt(void);

static inline void load_idt(void);

static inline struct idt_gate make_idt_gate(void (*addr)(void), enum idt_gate_type gate_type);

static inline void send_init_ipi_x2apic(uint32_t apic_id);

static inline void send_startup_ipi_x2apic(uint32_t apic_id, uint8_t vector);

static inline void send_init_ipi_xapic(uint32_t apic_base, uint8_t apic_id);

static inline void send_startup_ipi_xapic(uint32_t apic_base, uint8_t apic_id, uint8_t vector);

static inline uint64_t rdmsr(uint32_t addr);

static inline void wrmsr(uint32_t addr, uint64_t content);

static inline void enable_x2apic(void);

static inline void cpuid(uint32_t eax, uint32_t *eax_out, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);

static union gdt_elem gdt[2] = {
    [1].code = {
        .c = 1,
        .one = 1,
        .one2 = 1,
        .dpl = 0,
        .p = 1,
        .l = 1,
        .d = 0,
    }
};

static const struct seg_sel cs = {
    .rpl = 0,
    .ti = 0,
    .si = 1
};

static struct idt_gate idt[33];

void cpuid(uint32_t eax, uint32_t *eax_out, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
    __asm__ volatile(
        "mov %4, %%eax;"
        "cpuid;"
        "mov %%eax, %0;"
        "mov %%ebx, %1;"
        "mov %%ecx, %2;"
        "mov %%edx, %3;"
        : "=m"(eax_out), "=m"(ebx), "=m"(ecx), "=m"(edx)
        : "m"(eax)
        : "eax", "ebx", "ecx", "edx");
}

void enable_x2apic(void)
{
    uint64_t apic = rdmsr(0x1b);
    apic |= 0b110000000000;
    wrmsr(0x1b, apic);
}

uint64_t rdmsr(uint32_t addr)
{
    uint32_t low, high;

    __asm__ volatile(
        "movl %2, %%ecx;"
        "rdmsr;"
        "movl %%eax, %0;"
        "movl %%edx, %1;"
        : "=m"(low), "=m"(high)
        : "m"(addr)
        : "ecx", "eax", "edx");

    return (uint64_t)low | ((uint64_t)high << 32);
}

void wrmsr(uint32_t addr, uint64_t content)
{
    uint32_t low = (uint32_t)content;
    uint32_t high = content >> 32;

    __asm__ volatile(
        "movl %0, %%ecx;"
        "movl %1, %%eax;"
        "movl %2, %%edx;"
        "wrmsr;"
        :
        : "m"(addr), "m"(low), "m"(high)
        : "ecx", "eax", "edx");
}

void load_gdt(void)
{
    struct desc_reg gdtr = {
        .limit = sizeof(gdt) - 1,
        .base_addr = (uint64_t)gdt
    };

    __asm__ volatile("lgdt %0" ::"m"(gdtr));

    // Reload CS register to point
    // to the new code segment in our GDT.

    __asm__ volatile goto(
        "pushq %0;"
        "movabs $%l1, %%rax;"
        "push %%rax;"
        "lretq;"
        :
        : "m"(cs)
        : "rax"
        : dummy);
dummy:;
}

void load_idt(void)
{
    struct desc_reg idtr = {
        .limit = sizeof(idt) - 1,
        .base_addr = (uint64_t)idt
    };

    __asm__ volatile("lidt %0" ::"m"(idtr));
}

struct idt_gate make_idt_gate(void (*addr)(void), enum idt_gate_type gate_type)
{
    return (struct idt_gate){
        .target_off1 = (uint64_t)addr & 0xffff,
        .target_sel = *(const uint16_t *)&cs,
        .ist = 0,
        .type = gate_type,
        .zero = 0,
        .dpl = 0,
        .p = 1,
        .target_off2 = ((uint64_t)addr & 0xffffffffffff0000) >> 16
    };
}

void send_init_ipi_x2apic(uint32_t apic_id)
{
    struct icr_x2apic icr = {
        .vector = 0,
        .del_mode = 0b101,
        .dest_mode = 0,
        .level = 1,
        .trigger_mode = 0,
        .dest_shorthand = 0b00,
        .dest = apic_id
    };

    wrmsr(0x830, *(uint64_t *)&icr);
}

void send_startup_ipi_x2apic(uint32_t apic_id, uint8_t vector)
{
    struct icr_x2apic icr = {
        .vector = vector,
        .del_mode = 0b110,
        .dest_mode = 0,
        .level = 1,
        .trigger_mode = 0,
        .dest_shorthand = 0b00,
        .dest = apic_id
    };

    wrmsr(0x830, *(uint64_t *)&icr);
}

void send_init_ipi_xapic(uint32_t apic_base, uint8_t apic_id)
{
    struct icr_xapic_high high = {
        .dest = apic_id
    };

    struct icr_xapic_low low = {
        .vector = 0,
        .del_mode = 0b101,
        .dest_mode = 0,
        .level = 1,
        .trigger_mode = 0,
        .dest_shorthand = 0b00,
    };

    __asm__ volatile(
        "movl %0, (%1);"
        "movl %2, (%3);"
        :
        : "r"(high), "r"(apic_base + 310), "r"(low), "r"(apic_base + 300));
}

void send_startup_ipi_xapic(uint32_t apic_base, uint8_t apic_id, uint8_t vector)
{
    struct icr_xapic_high high = {
        .dest = apic_id
    };

    struct icr_xapic_low low = {
        .vector = vector,
        .del_mode = 0b110,
        .dest_mode = 0,
        .level = 1,
        .trigger_mode = 0,
        .dest_shorthand = 0b00,
    };

    __asm__ volatile(
        "movl %0, (%1);"
        "movl %2, (%3);"
        :
        : "r"(high), "r"(apic_base + 310), "r"(low), "r"(apic_base + 300));
}
