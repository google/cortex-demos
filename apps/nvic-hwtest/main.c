#include <nvic.h>

volatile int counter = 0;

void simple_handler(void) {
    ++counter;
}

int main(void) {
    nvic_init();
    nvic_set_handler(IRQ_PENDSV, simple_handler);
    nvic_enable_irqs();

    nvic_set_handler(1, simple_handler);
    nvic_enable_irq(1);

    nvic_irqset(IRQ_PENDSV);
    nvic_irqset(1);
    while (1);
}
