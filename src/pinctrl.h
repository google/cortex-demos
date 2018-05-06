#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int pinctrl_request_option(uint32_t port, unsigned int gpio_n, int option);

#ifdef __cplusplus
}  /* extern "C" */
#endif
