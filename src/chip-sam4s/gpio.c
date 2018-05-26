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
