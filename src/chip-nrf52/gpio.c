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
#include "memio.h"

#define GPIO_BASE       (0x50000000)
#define GPIO_OUT        (GPIO_BASE + 0x504)
#define GPIO_OUTSET        (GPIO_BASE + 0x508)
#define GPIO_OUTCLR        (GPIO_BASE + 0x50c)
#define GPIO_IN     (GPIO_BASE + 0x510)

#define GPIO_DIR        (GPIO_BASE + 0x514)
#define GPIO_DIRSET        (GPIO_BASE + 0x518)
#define GPIO_DIRCLR        (GPIO_BASE + 0x51c)


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

uint32_t gpio_get(uint32_t port) {
    (void)port;

    return raw_read32(GPIO_IN);
}

int gpio_set_option(uint32_t port, uint32_t mask, enum gpio_option opt) {
    (void)port;
    int ret = 0;
    switch (opt) {
    case GPIO_OPT_OUTPUT:
        raw_write32(GPIO_DIRSET, mask);
        break;
    case GPIO_OPT_INPUT:
        raw_write32(GPIO_DIRCLR, mask);
        break;
    default:
        ret = -1;
    }

    return ret;
}
