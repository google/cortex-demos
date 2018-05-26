#include "gpio.h"

#include "cutils.h"
#include "memio.h"

#include "sam4s/gpio.h"

#define PORTA_BASE      (0x400E0E00)
#define PORTB_BASE      (0x400E1000)
#define PORTC_BASE      (0x400E1200)

/* Register offsets in port */
#define PIO_SODR        (0x30)
#define PIO_CODR        (0x34)
#define PIO_ODSR        (0x38)
#define PIO_PDSR        (0x3c)

/* For production compilation this can be defined to no-op */
#ifndef gpioASSERT
#define gpioASSERT(cond)    if (!(cond)) { return -1; }
#endif

const uint32_t ports[] = {
    PORTA_BASE,
    PORTB_BASE,
    PORTC_BASE,
};

int gpio_set(uint32_t port, uint32_t mask) {
    gpioASSERT(port < ARRAY_SIZE(ports));

    raw_write32(ports[port] + PIO_SODR, mask);

    return 0;
}

int gpio_clear(uint32_t port, uint32_t mask) {
    gpioASSERT(port < ARRAY_SIZE(ports));

    raw_write32(ports[port] + PIO_CODR, mask);

    return 0;
}

int gpio_toggle(uint32_t port, uint32_t mask) {
    gpioASSERT(port < ARRAY_SIZE(ports));

    const uint32_t port_base = ports[port];
    uint32_t status = raw_read32(port_base + PIO_ODSR);
    raw_write32(port_base + PIO_SODR, mask ^ status);
    raw_write32(port_base + PIO_CODR, mask ^ ~status);
    return 0;
}

uint32_t gpio_get(uint32_t port) {
    gpioASSERT(port < ARRAY_SIZE(ports));

    return raw_read32(ports[port] + PIO_PDSR);
}
