#include "nvic.h"

#include "chip.h"
#include "cutils.h"
#include "memio.h"

static irq_handler_func_t vector_table[CHIP_NUM_IRQS - IRQ_OFFSET];

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
