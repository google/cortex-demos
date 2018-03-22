#include <nvic.h>

volatile int counter = 0;

void simple_handler(void) {
    ++counter;
}

int main(void) {
    nvic_set_handler(IRQ_SVCALL, simple_handler);

    while (1);
}
