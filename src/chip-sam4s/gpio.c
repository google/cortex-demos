/*******************************************************************************
    Copyright 2018 Google LLC

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

#include "gpio.h"

#include "cutils.h"
#include "memio.h"

#include "sam4s/gpio.h"

#define PORTA_BASE      (0x400E0E00)
#define PORTB_BASE      (0x400E1000)
#define PORTC_BASE      (0x400E1200)

/* Register offsets in port */
#define PIO_PER        (0x00)
#define PIO_PDR        (0x04)
#define PIO_PSR        (0x08)
#define PIO_OER        (0x10)
#define PIO_ODR        (0x14)
#define PIO_OSR        (0x18)
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

int gpio_set_option(uint32_t port, uint32_t mask, enum gpio_option opt) {
    gpioASSERT(port < ARRAY_SIZE(ports));

    int ret = 0;
    const uint32_t port_base = ports[port];
    switch (opt) {
    case GPIO_OPT_OUTPUT:
        raw_write32(port_base + PIO_PER, mask);
        raw_write32(port_base + PIO_OER, mask);
        break;
    case GPIO_OPT_INPUT:
        raw_write32(port_base + PIO_PER, mask);
        raw_write32(port_base + PIO_ODR, mask);
        break;
    default:
        ret = -1;
        break;
    }

    return ret;
}
