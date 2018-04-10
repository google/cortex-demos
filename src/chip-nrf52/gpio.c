#include "gpio.h"
#include "memio.h"

#define GPIO_BASE       (0x50000000)
#define GPIO_OUT        (GPIO_BASE + 0x504)
#define GPIO_OUTSET        (GPIO_BASE + 0x508)
#define GPIO_OUTCLR        (GPIO_BASE + 0x50c)


int gpio_set(uint32_t port, uint32_t mask) {
    (void)port;

    raw_write32(GPIO_OUTSET, mask);
    return 0;
}

int gpio_clear(uint32_t port, uint32_t mask) {
    (void)port;

    raw_write32(GPIO_OUTCLR, mask);
    return 0;
}

int gpio_toggle(uint32_t port, uint32_t mask) {
    (void)port;

    uint32_t value = raw_read32(GPIO_OUT);
    raw_write32(GPIO_OUT, value ^ mask);
    return 0;
}
