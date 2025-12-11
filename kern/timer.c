#include <inc/types.h>
#include <inc/assert.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/mmu.h>
#include <inc/stdio.h>
#include <inc/x86.h>
#include <inc/uefi.h>
#include <kern/timer.h>
#include <kern/kclock.h>
#include <kern/picirq.h>
#include <kern/trap.h>

#define kilo      (1000ULL)
#define Mega      (kilo * kilo)
#define Giga      (kilo * Mega)
#define Tera      (kilo * Giga)
#define Peta      (kilo * Tera)
#define ULONG_MAX ~0UL

#if LAB <= 6
/* Early variant of memory mapping that does 1:1 aligned area mapping
 * in 2MB pages. You will need to reimplement this code with proper
 * virtual memory mapping in the future. */
void *
mmio_map_region(physaddr_t pa, size_t size) {
    void map_addr_early_boot(uintptr_t addr, uintptr_t addr_phys, size_t sz);
    const physaddr_t base_2mb = 0x200000;
    uintptr_t org = pa;
    size += pa & (base_2mb - 1);
    size += (base_2mb - 1);
    pa &= ~(base_2mb - 1);
    size &= ~(base_2mb - 1);
    map_addr_early_boot(pa, pa, size);
    return (void *)org;
}
void *
mmio_remap_last_region(physaddr_t pa, void *addr, size_t oldsz, size_t newsz) {
    return mmio_map_region(pa, newsz);
}
#endif

struct Timer timertab[MAX_TIMERS];
struct Timer *timer_for_schedule;

struct Timer timer_hpet0 = {
        .timer_name = "hpet0",
        .timer_init = hpet_init,
        .get_cpu_freq = hpet_cpu_frequency,
        .enable_interrupts = hpet_enable_interrupts_tim0,
        .handle_interrupts = hpet_handle_interrupts_tim0,
};

struct Timer timer_hpet1 = {
        .timer_name = "hpet1",
        .timer_init = hpet_init,
        .get_cpu_freq = hpet_cpu_frequency,
        .enable_interrupts = hpet_enable_interrupts_tim1,
        .handle_interrupts = hpet_handle_interrupts_tim1,
};

struct Timer timer_acpipm = {
        .timer_name = "pm",
        .timer_init = acpi_enable,
        .get_cpu_freq = pmtimer_cpu_frequency,
};

/* ACPI Spec sec. 5.2.5 "Root System Description Pointer (RSDP)": The RSDP structure
 * is located in the system's memory address space. On UEFI-enabled systems (sec. 5.2.5.2),
 * a pointer to the RSDP structure exists within the EFI System Table, and the OS loader
 * must retrieve it before ExitBootServices. In our environment, LOADER_PARAMS.ACPIRoot
 * holds this RSDP physical address. The RSDP structure (sec. 5.2.5.3, Table 5.3) contains
 * pointers to either RSDT (32-bit addresses, sec. 5.2.7) or XSDT (64-bit addresses,
 * sec. 5.2.8), which are arrays of system description table addresses. */
RSDP *
get_rsdp(void) {
    physaddr_t pa = uefi_lp->ACPIRoot;
    return mmio_map_region(pa, sizeof(RSDP));
}

/* Enable ACPI mode by writing AcpiEnable value to SMI_CommandPort (both fields
 * are defined in FADT, ACPI Spec sec. 5.2.9, Table 5.9). Wait until the SCI_EN
 * bit (bit 0) in PM1aControlBlock register is set, indicating ACPI mode is active. */
void
acpi_enable(void) {
    FADT *fadt = get_fadt();
    outb(fadt->SMI_CommandPort, fadt->AcpiEnable);
    while ((inw(fadt->PM1aControlBlock) & 1) == 0) /* nothing */
        ;
}

/* ACPI Spec sec. 5.2.5.3 "Root System Description Pointer (RSDP) Structure" (Table 5.3):
 * The RSDP signature must be "RSD PTR " (8 bytes, including trailing blank). The RSDP
 * contains pointers to RSDT (32-bit, sec. 5.2.7) and/or XSDT (64-bit, sec. 5.2.8).
 * Sec. 5.2.8 states: "An ACPI-compatible OS must use the XSDT if present." */
static void *
find_acpi_table(const char *sign) {
    /*
     * This function performs lookup of ACPI table by its signature
     * and returns valid pointer to the table mapped somewhere.
     *
     * It is a good idea to checksum tables before using them.
     *
     * HINT: Use mmio_map_region/mmio_remap_last_region
     * before accessing table addresses
     * (Why mmio_remap_last_region is requrired?)
     * HINT: RSDP address is stored in uefi_lp->ACPIRoot
     * HINT: You may want to distunguish RSDT/XSDT
     */
    // LAB 5: Your code here:
    RSDP *rsdp = get_rsdp();
    /* ACPI Spec sec. 5.2.5.3, Table 5.3: Signature field must be "RSD PTR " (with trailing blank). */
    if (strncmp(rsdp->Signature, "RSD PTR ", sizeof rsdp->Signature) != 0) return NULL;

    /* ACPI Spec sec. 5.2.5.3: XSDT address is only valid when Revision >= 2. The XSDT
     * (sec. 5.2.8, Table 5.8) provides identical functionality to RSDT but accommodates
     * 64-bit physical addresses. Sec. 5.2.8: "An ACPI-compatible OS must use the XSDT
     * if present." For backward compatibility, RsdtAddress (32-bit) may also be present. */
    XSDT *xsdt = NULL;
    RSDT *rsdt = NULL;

    if (rsdp->Revision >= 2 && rsdp->XsdtAddress) {
        physaddr_t pa = rsdp->XsdtAddress;
        xsdt = mmio_map_region(pa, sizeof(ACPISDTHeader));
        xsdt = mmio_remap_last_region(pa, xsdt, sizeof(ACPISDTHeader), xsdt->h.Length);
    }

    if (rsdp->RsdtAddress) {
        physaddr_t pa = rsdp->RsdtAddress;
        rsdt = mmio_map_region(pa, sizeof(ACPISDTHeader));
        rsdt = mmio_remap_last_region(pa, rsdt, sizeof(ACPISDTHeader), rsdt->h.Length);
    }

    /* ACPI Spec sec. 5.2.8, Table 5.8: XSDT contains an array of 64-bit physical addresses
     * (Entry field, 8*n bytes starting at offset 36) pointing to DESCRIPTION_HEADERs.
     * Sec. 5.2.8: "OSPM assumes at least the DESCRIPTION_HEADER is addressable, and then
     * can further address the table based upon its Length field." */
    if (xsdt) {
        size_t count = (xsdt->h.Length - sizeof(ACPISDTHeader)) / sizeof(uint64_t);
        uint64_t *ents = (uint64_t *)((uint8_t *)xsdt + sizeof(ACPISDTHeader));
        for (size_t i = 0; i < count; i++) {
            physaddr_t pa = ents[i];
            ACPISDTHeader *h = mmio_map_region(pa, sizeof(ACPISDTHeader));
            /* ACPI Spec sec. 5.2.6 "System Description Table Header" (Table 5.4): The Signature
             * field (4 bytes at offset 0) is "The ASCII string representation of the table
             * identifier." */
            if (strncmp(h->Signature, sign, 4) != 0) continue;
            h = mmio_remap_last_region(pa, h, sizeof(ACPISDTHeader), h->Length);
            /* ACPI Spec sec. 5.2.6, Table 5.4: "Checksum: The entire table, including the
             * checksum field, must add to zero to be considered valid." */
            uint8_t sum = 0;
            uint8_t *p = (uint8_t *)h;
            for (size_t j = 0; j < h->Length; j++) sum += p[j];
            if (sum != 0) continue;
            return h;
        }
    }

    /* ACPI Spec sec. 5.2.7, Table 5.7: RSDT contains an array of 32-bit physical addresses
     * (Entry field, 4*n bytes starting at offset 36) pointing to DESCRIPTION_HEADERs.
     * Used for backward compatibility with ACPI 1.0 systems. */
    if (rsdt) {
        size_t count = (rsdt->h.Length - sizeof(ACPISDTHeader)) / sizeof(uint32_t);
        uint32_t *ents = (uint32_t *)((uint8_t *)rsdt + sizeof(ACPISDTHeader));
        for (size_t i = 0; i < count; i++) {
            physaddr_t pa = ents[i];
            ACPISDTHeader *h = mmio_map_region(pa, sizeof(ACPISDTHeader));
            if (strncmp(h->Signature, sign, 4) != 0) continue;
            h = mmio_remap_last_region(pa, h, sizeof(ACPISDTHeader), h->Length);
            /* ACPI Spec sec. 5.2.6: Verify checksum (entire table must sum to zero). */
            uint8_t sum = 0;
            uint8_t *p = (uint8_t *)h;
            for (size_t j = 0; j < h->Length; j++) sum += p[j];
            if (sum != 0) continue;
            return h;
        }
    }

    return NULL;
}

/* Obtain and map FADT ACPI table address.
 * ACPI Spec sec. 5.2.9 "Fixed ACPI Description Table (FADT)": The FADT defines various
 * fixed hardware ACPI information vital to an ACPI-compatible OS, such as base addresses
 * for hardware register blocks (PM1a_EVT_BLK, PM_TMR_BLK, etc.). Table 5.9 specifies
 * that the table signature is "FACP" (not "FADT") - "This signature predates ACPI 1.0,
 * explaining the mismatch with this table's name." */
FADT *
get_fadt(void) {
    FADT *fadt = (FADT *)find_acpi_table("FACP");
    if (!fadt) panic("FADT (FACP) table not found\n");
    return fadt;
}

/* Obtain and map HPET ACPI table address.
 * HPET Spec sec. 3.2.4 "The ACPI 2.0 HPET Description Table (HPET)": The HPET
 * Description Table is a means to report the Base Addresses of each Event Timer Block
 * early in the OS boot process. The table signature is "HPET" (4 bytes at offset 0).
 * The table contains the base address of the HPET register block in the BASE_ADDRESS
 * field (12-byte ACPI Generic Address Structure, GAS format, see Table 3 in sec. 3.2.4). */
HPET *
get_hpet(void) {
    // LAB 5: Your code here
    // (use find_acpi_table)
    HPET *hpet = (HPET *)find_acpi_table("HPET");
    if (!hpet) panic("HPET table not found\n");
    return hpet;
}

/* Getting physical HPET timer address from its table. */
HPETRegister *
hpet_register(void) {
    HPET *hpet_timer = get_hpet();
    if (!hpet_timer->address.address) panic("hpet is unavailable\n");

    uintptr_t paddr = hpet_timer->address.address;
    return mmio_map_region(paddr, sizeof(HPETRegister));
}

/* Debug HPET timer state. */
void
hpet_print_struct(void) {
    HPET *hpet = get_hpet();
    assert(hpet != NULL);
    cprintf("signature = %s\n", (hpet->h).Signature);
    cprintf("length = %08x\n", (hpet->h).Length);
    cprintf("revision = %08x\n", (hpet->h).Revision);
    cprintf("checksum = %08x\n", (hpet->h).Checksum);

    cprintf("oem_revision = %08x\n", (hpet->h).OEMRevision);
    cprintf("creator_id = %08x\n", (hpet->h).CreatorID);
    cprintf("creator_revision = %08x\n", (hpet->h).CreatorRevision);

    cprintf("hardware_rev_id = %08x\n", hpet->hardware_rev_id);
    cprintf("comparator_count = %08x\n", hpet->comparator_count);
    cprintf("counter_size = %08x\n", hpet->counter_size);
    cprintf("reserved = %08x\n", hpet->reserved);
    cprintf("legacy_replacement = %08x\n", hpet->legacy_replacement);
    cprintf("pci_vendor_id = %08x\n", hpet->pci_vendor_id);
    cprintf("hpet_number = %08x\n", hpet->hpet_number);
    cprintf("minimum_tick = %08x\n", hpet->minimum_tick);

    cprintf("address_structure:\n");
    cprintf("address_space_id = %08x\n", (hpet->address).address_space_id);
    cprintf("register_bit_width = %08x\n", (hpet->address).register_bit_width);
    cprintf("register_bit_offset = %08x\n", (hpet->address).register_bit_offset);
    cprintf("address = %08lx\n", (unsigned long)(hpet->address).address);
}

static volatile HPETRegister *hpetReg;
/* HPET timer period (in femtoseconds) */
static uint64_t hpetFemto = 0;
/* HPET timer frequency */
static uint64_t hpetFreq = 0;

/* HPET timer initialisation */
void
hpet_init() {
    if (hpetReg == NULL) {
        nmi_disable();
        hpetReg = hpet_register();
        uint64_t cap = hpetReg->GCAP_ID;
        /* HPET Spec sec. 2.3.4 "General Capabilities and ID Register":
         *  - Bits 63:32 (COUNTER_CLK_PERIOD): "Main Counter Tick Period: This read-only
         *    field indicates the period at which the counter increments in femtoseconds
         *    (10^-15 seconds)."
         *  - Bit 15 (LEG_RT_CAP): "LegacyReplacement Route Capable: If this bit is a 1,
         *    it indicates that the hardware supports the LegacyReplacement Interrupt
         *    Route option." (See sec. 2.4.2.1 for routing details) */
        hpetFemto = (uintptr_t)(cap >> 32);
        if (!(cap & HPET_LEG_RT_CAP)) panic("HPET has no LegacyReplacement mode");

        hpetFreq = (1 * Peta) / hpetFemto;
        /* HPET Spec sec. 2.3.5 "General Configuration Register", bit 0 (ENABLE_CNF):
         *  "Overall Enable: This bit must be set to enable any of the timers to generate
         *  interrupts. If this bit is 0, then the main counter will halt (will not increment)
         *  and no interrupts will be caused by any of these timers." */
        hpetReg->GEN_CONF |= HPET_ENABLE_CNF;
        nmi_enable();
    }
}

/* HPET register contents debugging. */
void
hpet_print_reg(void) {
    cprintf("GCAP_ID = %016lx\n", (unsigned long)hpetReg->GCAP_ID);
    cprintf("GEN_CONF = %016lx\n", (unsigned long)hpetReg->GEN_CONF);
    cprintf("GINTR_STA = %016lx\n", (unsigned long)hpetReg->GINTR_STA);
    cprintf("MAIN_CNT = %016lx\n", (unsigned long)hpetReg->MAIN_CNT);
    cprintf("TIM0_CONF = %016lx\n", (unsigned long)hpetReg->TIM0_CONF);
    cprintf("TIM0_COMP = %016lx\n", (unsigned long)hpetReg->TIM0_COMP);
    cprintf("TIM0_FSB = %016lx\n", (unsigned long)hpetReg->TIM0_FSB);
    cprintf("TIM1_CONF = %016lx\n", (unsigned long)hpetReg->TIM1_CONF);
    cprintf("TIM1_COMP = %016lx\n", (unsigned long)hpetReg->TIM1_COMP);
    cprintf("TIM1_FSB = %016lx\n", (unsigned long)hpetReg->TIM1_FSB);
    cprintf("TIM2_CONF = %016lx\n", (unsigned long)hpetReg->TIM2_CONF);
    cprintf("TIM2_COMP = %016lx\n", (unsigned long)hpetReg->TIM2_COMP);
    cprintf("TIM2_FSB = %016lx\n", (unsigned long)hpetReg->TIM2_FSB);
}

/* HPET Spec sec. 2.3.7 "Main Counter Register": "Reads to this register return the
 *  current value of the main counter. Reads to this register are monotonic. No two
 *  consecutive reads will return the same value, except when the reads happen at less
 *  than the resolution of the counter or if the counter has rolled over." */
uint64_t
hpet_get_main_cnt(void) {
    return hpetReg->MAIN_CNT;
}

/* - Configure HPET timer 0 to trigger every 0.5 seconds on IRQ_TIMER line
 * - Configure HPET timer 1 to trigger every 1.5 seconds on IRQ_CLOCK line
 *
 * HINT To be able to use HPET as PIT replacement consult
 *      LegacyReplacement functionality in HPET spec.
 * HINT Don't forget to unmask interrupt in PIC */
void
hpet_enable_interrupts_tim0(void) {
    /* HPET Spec sec. 2.4.2.1 "Mapping Option #1: LegacyReplacement Option":
     *  "If the Legacy Replacement Route bit (LEG_RT_CNF) is set ('1'), the following
     *  mapping is forced: Timer 0 → IRQ0 (8259) / IRQ2 (APIC), Timer 1 → IRQ8."
     *  This allows HPET to replace 8254 PIT (IRQ0) and RTC periodic interrupt (IRQ8). */
    hpet_init();

    /* HPET Spec sec. 2.3.5: Halt main counter (clear ENABLE_CNF) before reprogramming
     *  to avoid race conditions (see sec. 2.3.9.2.2 "Periodic Mode"). */
    hpetReg->GEN_CONF &= ~HPET_ENABLE_CNF;

    /* HPET Spec sec. 2.3.5, bit 1 (LEG_RT_CNF): "LegacyReplacement Route: If the
     *  ENABLE_CNF bit and the LEG_RT_CNF bit are both set, then the interrupts will
     *  be routed as follows: Timer 0 will be routed to IRQ0 in Non-APIC or IRQ2 in
     *  the I/O APIC, Timer 1 will be routed to IRQ8." */
    hpetReg->GEN_CONF |= HPET_LEG_RT_CNF;

    /* HPET Spec sec. 2.3.8 "Timer N Configuration and Capabilities Register":
     *  - Bit 2 (TN_INT_ENB_CNF): "Timer n Interrupt Enable: This read/write bit must
     *    be set to enable timer n to cause an interrupt when the timer event fires."
     *  - Bit 3 (TN_TYPE_CNF): "Timer n Type: Writing a 1 to this bit enables the timer
     *    to generate a periodic interrupt." (See sec. 2.4.3.2 "Periodic Mode")
     *  - Bit 6 (TN_VAL_SET_CNF): "Timer n Value Set: Software uses this read/write bit
     *    only for timers that have been set to periodic mode. By writing this bit to a 1,
     *    the software is then allowed to directly set a periodic timer's accumulator."
     *    (See sec. 2.3.9.2.2 for usage model) */
    uint64_t conf = hpetReg->TIM0_CONF;
    conf |= HPET_TN_INT_ENB_CNF; /* enable interrupts */
    conf |= HPET_TN_TYPE_CNF;    /* periodic mode */
    hpetReg->TIM0_CONF = conf;

    /* HPET Spec sec. 2.3.9.2.2 "Periodic Mode": "To make the periodic mode work properly,
     *  the main counter is typically written with a value of 0 so that the first interrupt
     *  occurs at the right point for the comparator." The comparator value (TIM0_COMP)
     *  represents the interval between interrupts: for 0.5 seconds, delta = hpetFreq * 0.5. */
    uint64_t delta = hpetFreq / 2;
    hpetReg->MAIN_CNT = 0;
    hpetReg->TIM0_COMP = delta;

    /* Re-enable main counter and unmask IRQ line on PIC. */
    hpetReg->GEN_CONF |= HPET_ENABLE_CNF;
    pic_irq_unmask(IRQ_TIMER);
}

void
hpet_enable_interrupts_tim1(void) {
    hpet_init();

    /* HPET Spec sec. 2.3.5: Halt main counter before reprogramming (see sec. 2.3.9.2.2). */
    hpetReg->GEN_CONF &= ~HPET_ENABLE_CNF;

    /* LegacyReplacement routing (LEG_RT_CNF) already enabled by timer 0 configuration.
     *  Timer 1 will be routed to IRQ8 per HPET Spec sec. 2.4.2.1. */

    /* HPET Spec sec. 2.3.8: Configure timer 1 for periodic interrupts (see comments
     *  in hpet_enable_interrupts_tim0 for bit definitions). */
    uint64_t conf = hpetReg->TIM1_CONF;
    conf |= HPET_TN_INT_ENB_CNF;
    conf |= HPET_TN_TYPE_CNF;
    conf |= HPET_TN_VAL_SET_CNF;
    hpetReg->TIM1_CONF = conf;

    /* HPET Spec sec. 2.3.9.2.2: Program comparator for 1.5 seconds interval. */
    uint64_t delta = (hpetFreq * 3) / 2;
    hpetReg->MAIN_CNT = 0;
    hpetReg->TIM1_COMP = delta;

    hpetReg->GEN_CONF |= HPET_ENABLE_CNF;
    pic_irq_unmask(IRQ_CLOCK);
}

void
hpet_handle_interrupts_tim0(void) {
    pic_send_eoi(IRQ_TIMER);
}

void
hpet_handle_interrupts_tim1(void) {
    pic_send_eoi(IRQ_CLOCK);
}

/* Calculate CPU frequency in Hz by comparing TSC delta with HPET main counter delta.
 * HPET Spec sec. 2.3.4 (COUNTER_CLK_PERIOD) and 2.3.7 (Main Counter Register):
 *  - COUNTER_CLK_PERIOD (bits 63:32 of GCAP_ID) gives \"Main Counter Tick Period ...
 *    in femptoseconds (10^-15 seconds)\"; hpetFreq = 1 / period.
 *  - The main counter is a free-running up-counter that increments at this rate and
 *    reads are monotonic (no two consecutive reads return a smaller value).
 * Измеряя при этом интервал по MAIN_CNT и по TSC, мы получаем отношение частот и,
 * умножая дельту TSC на известную частоту HPET, восстанавливаем частоту CPU. */
uint64_t
hpet_cpu_frequency(void) {
    static uint64_t cpu_freq;

    if (!cpu_freq) {
        hpet_init();

        /* Intel SDM Vol. 2A, sec. 3-166 "CLI—Clear Interrupt Flag": "In most cases,
         * CLI clears the IF flag in the EFLAGS register and no other flags are affected.
         * Clearing the IF flag causes the processor to ignore maskable external interrupts."
         * We disable interrupts during measurement to ensure atomicity of the TSC/HPET
         * counter reads. */
        uint64_t interrupts_enabled = read_rflags() & FL_IF;
        asm volatile("cli");

        uint64_t c1 = hpet_get_main_cnt();
        /* Intel SDM Vol. 2B, sec. 4-558 "RDTSC—Read Time-Stamp Counter": "Reads the
         * current value of the processor's time-stamp counter (a 64-bit MSR) into the
         * EDX:EAX registers." The instruction "is not a serializing instruction" but
         * provides a monotonic counter value. */
        uint64_t t1 = read_tsc();

        /* Wait until at least 1/10 second has passed on HPET to get a
         * reasonably accurate measurement. */
        uint64_t threshold = hpetFreq / 10;
        uint64_t c2;
        do {
            c2 = hpet_get_main_cnt();
            asm volatile("pause");
        } while (c2 - c1 < threshold);
        uint64_t t2 = read_tsc();

        /* Intel SDM Vol. 2A, sec. 4-670 "STI—Set Interrupt Flag": Sets the IF flag in
         * EFLAGS, enabling maskable external interrupts. We restore the previous interrupt
         * state that was saved before CLI. */
        if (interrupts_enabled) asm volatile("sti");

        uint64_t timer_delta = c2 - c1;
        uint64_t tsc_delta = t2 - t1;
        cpu_freq = (tsc_delta * hpetFreq) / timer_delta;
    }

    return cpu_freq;
}

uint32_t
pmtimer_get_timeval(void) {
    FADT *fadt = get_fadt();
    return inl(fadt->PMTimerBlock);
}

/* Calculate CPU frequency in Hz with the help with ACPI PowerManagement timer.
 * HINT Use pmtimer_get_timeval function and do not forget that ACPI PM timer
 *      can be 24-bit or 32-bit. */
uint64_t
pmtimer_cpu_frequency(void) {
    static uint64_t cpu_freq;

    if (!cpu_freq) {
        /* The ACPI Power Management Timer is defined as a free-running
         * up-counter that "increments at a fixed frequency of 3.579545 MHz"
         * and may be implemented as a 24- or 32-bit counter (ACPI Spec v6.x,
         * sec. 4.8.3.1 "Power Management Timer"). We compare its delta with
         * the TSC delta to derive CPU frequency. */
        /* Intel SDM Vol. 2A, sec. 3-166 "CLI": Disable interrupts during measurement
         * to ensure atomicity of counter reads. */
        uint64_t interrupts_enabled = read_rflags() & FL_IF;
        asm volatile("cli");

        uint64_t t1 = pmtimer_get_timeval();
        /* Intel SDM Vol. 2B, sec. 4-558 "RDTSC": Read TSC counter value. */
        uint64_t tsc1 = read_tsc();



        /* Intel SDM Vol. 2A, sec. 4-670 "STI": Restore interrupt state. */
        if (interrupts_enabled) asm volatile("sti");

        uint64_t timer_delta = 0;
        do {
            asm volatile("pause");
            uint64_t t2 = pmtimer_get_timeval();
            if (t2 > t1) {
                /* No overflow. */
                timer_delta += t2 - t1;
            } else if (t1 - t2 <= 0x00FFFFFF) {
                /* Overflow assuming 24-bit timer. */
                timer_delta += 0x00FFFFFF - t1 + t2;
            } else {
                /* Overflow assuming 32-bit timer. */
                timer_delta += UINT32_MAX - t1 + t2;
            }
            t1 = t2;
        } while (timer_delta < (hpetFreq / 10));



        cpu_freq = (read_tsc() - tsc1) * PM_FREQ / timer_delta;
    }

    return cpu_freq;
}
