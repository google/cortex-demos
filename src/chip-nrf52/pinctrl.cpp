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

#include "pinctrl.hpp"

#include "gpio.h"
#include "nrf52/pinctrl.hpp"

namespace pinctrl {

using pf = function;

int request_function(int pin_function) {
    int pin = get_pin(pin_function);
    if (pin == -1) {
        return -1;
    }

    int ret = pin;
    switch (pin_function) {
        case pf::UARTE0_RXD:
        case pf::UARTE1_RXD:
        case pf::UARTE0_CTS:
        case pf::UARTE1_CTS:
            gpio_set_option(0, (1 << pin), GPIO_OPT_INPUT);
            break;
        case pf::UARTE0_TXD:
        case pf::UARTE1_TXD:
        case pf::UARTE0_RTS:
        case pf::UARTE1_RTS:
            gpio_set_option(0, (1 << pin), GPIO_OPT_OUTPUT);
            gpio_set(0, (1 << pin));
            break;
        default:
            break;
    }

    return ret;
}

}  // namespace pinctrl
