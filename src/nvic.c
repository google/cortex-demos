#include "nvic.h"

#include <stdint.h>

#include "chip.h"
#include "cutils.h"
#include "memio.h"
#include "syscontrol.h"
#include "types.h"

#define SCB_ICSR        (0xe000ed04)
#define SCB_ICSR_NMIPENDSET     (1 << 31)
#define SCB_ICSR_PENDSVSET     (1 << 28)
#define SCB_ICSR_PENDSVCLR     (1 << 27)
#define SCB_ICSR_PENDSTSET      (1 << 26)
#define SCB_ICSR_PENDSTCLR      (1 << 25)

#define NVIC_IRQ_MASK(irq)    (1 << ((irq) & 0x1f))
#define NVIC_IRQ_REGN(irq)     ((irq) >> 5)

#define NVIC_ISER_BASE      (0xe000e100)
#define NVIC_ISER(n)        (NVIC_ISER_BASE + (n) * 4)

#define NVIC_ISPR_BASE      (0xe000e200)
#define NVIC_ISPR(n)      (NVIC_ISPR_BASE + (n) * 4)

static irq_handler_func_t __attribute__((aligned(CHIP_IRQ_TABLE_ALIGN))) vector_table[CHIP_NUM_IRQS - IRQ_OFFSET];

void nvic_init(void) {
    /* Only relocate top of the stack and reset handler */
    syscontrol_relocate_vt((ubase_t)vector_table, 1);
}

int nvic_set_handler(int irqn, irq_handler_func_t handler_func) {
    const size_t offset = irqn - IRQ_OFFSET;
    if (offset > ARRAY_SIZE(vector_table)) {
        return -1;
    }

    vector_table[offset] = handler_func;
    return 0;
}

int nvic_dispatch(int irqn) {
    const size_t offset = irqn - IRQ_OFFSET;
    if (offset > ARRAY_SIZE(vector_table)) {
        return -1;
    }

    vector_table[offset]();
    return 0;
}

int nvic_irqset(int irqn) {
    if (irqn < 0) {
        uint32_t setbit = 0;
        switch (irqn) {
            case IRQ_SYSTICK:
                setbit = SCB_ICSR_PENDSTSET;
                break;
            case IRQ_PENDSV:
                setbit = SCB_ICSR_PENDSVSET;
                break;
            case IRQ_NMI:
                setbit = SCB_ICSR_NMIPENDSET;
                break;
        }

        if (setbit) {
            raw_write32(SCB_ICSR, setbit);
        } else {
            return -1;
        }
    } else {
        raw_write32(NVIC_ISPR(NVIC_IRQ_REGN(irqn)), NVIC_IRQ_MASK(irqn));
    }

    return -1;
}

void nvic_enable_irqs() {
#ifndef CHIP_NATIVETEST
    __asm__ ("cpsie i");
#endif
}

void nvic_disable_irqs() {
#ifndef CHIP_NATIVETEST
    __asm__ ("cpsid i");
#endif
}

void nvic_enable_irq(int irqn) {
    raw_write32(NVIC_ISER(NVIC_IRQ_REGN(irqn)), NVIC_IRQ_MASK(irqn));
}
