#pragma once

#include <stdint.h>

/* This file defines the common GPIO API as implemented for all supported SoCs */

enum gpio_option {
    GPIO_OPT_TRI,
    GPIO_OPT_OUTPUT,
    GPIO_OPT_INPUT,
};

#ifdef __cplusplus
extern "C" {
#endif

/** Enable GPIOs in the given port
 *
 * @param[port] GPIO port
 * @param[mask] mask for GPIOs to set in the port
 */
int gpio_set(uint32_t port, uint32_t mask);

/** Disable GPIOs in the given port
 *
 * @param[port] GPIO port
 * @param[mask] mask for GPIOs to clear in the port
 */
int gpio_clear(uint32_t port, uint32_t mask);

/** Toggle GPIOs in the given port
 *
 * @param[port] GPIO port
 * @param[mask] mask for GPIOs to toggle in the port
 */
int gpio_toggle(uint32_t port, uint32_t mask);

/** Return GPIO values in the given port
 *
 * @param[port] GPIO port to read
 */
uint32_t gpio_get(uint32_t port);

int gpio_set_option(uint32_t port, uint32_t mask, enum gpio_option opt);

#ifdef __cplusplus
}
#endif
