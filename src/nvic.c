/*******************************************************************************
    Copyright 2018,2019 Google LLC

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        https://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*******************************************************************************/

#include "nvic.h"

#include <stdint.h>
#include <stdlib.h>

#include "chip.h"
#include "cutils.h"
#include "memio.h"
#include "syscontrol.h"

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

#define NVIC_ICER_BASE      (0xe000e180)
#define NVIC_ICER(n)        (NVIC_ICER_BASE + (n) * 4)

#define NVIC_ISPR_BASE      (0xe000e200)
#define NVIC_ISPR(n)      (NVIC_ISPR_BASE + (n) * 4)

static irq_handler_func_t __attribute__((aligned(CHIP_IRQ_TABLE_ALIGN))) vector_table[CHIP_NUM_IRQS - IRQ_OFFSET];

static void blocking_handler(void) {
#ifndef CHIP_NATIVETEST
    while (1);
#else
    abort();
#endif
}

void nvic_init(void) {
    for (size_t i = 0; i < ARRAY_SIZE(vector_table); ++i) {
        vector_table[i] = 0;
    }
    /* Only relocate top of the stack and reset handler */
    syscontrol_relocate_vt((uintptr_t)vector_table, 1);
    for (int i = IRQ_NMI; i < IRQ_IRQ0; ++i) {
        nvic_set_handler(i, blocking_handler);
    }
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

    if (!vector_table[offset]) {
        return -2;
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
#if defined(__arm__) && defined(__thumb__)
    __asm__("cpsie i");
#endif
}

void nvic_disable_irqs() {
#if defined(__arm__) && defined(__thumb__)
    __asm__("cpsid i");
#endif
}

void nvic_enable_irq(int irqn) {
    raw_write32(NVIC_ISER(NVIC_IRQ_REGN(irqn)), NVIC_IRQ_MASK(irqn));
}

void nvic_disable_irq(int irqn) {
    raw_write32(NVIC_ICER(NVIC_IRQ_REGN(irqn)), NVIC_IRQ_MASK(irqn));
}

irq_handler_func_t* nvic_get_table(void) {
    return vector_table;
}
