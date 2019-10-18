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
#include "memio.h"
#include "nrf52/pinctrl.hpp"

namespace {

using pf = pinctrl::function;

constexpr uint32_t uarte0_base = 0x40002000;
constexpr uint32_t uarte1_base = 0x40028000;

constexpr uint32_t twim0_base = 0x40003000;
constexpr uint32_t twim1_base = 0x40004000;

constexpr uint32_t twim_psel_offset(int func) {
    return 0x504 + 4 * func;
}

constexpr uint32_t uarte_psel_offset(int func) {
    return 0x504 + 4 * func;
}

void _configure_uarte(uint32_t base, int group) {
    constexpr uint32_t pin_disconnect_value = 0xffffffff;
    auto rts = pinctrl::get_pin(pf::UARTE_RTS + group);
    if (rts != -1) {
        gpio_set_option(0, (1 << rts), GPIO_OPT_OUTPUT);
        gpio_set(0, (1 << rts));
        raw_write32(base + uarte_psel_offset(pf::UARTE_RTS), rts);
    } else {
        raw_write32(base + uarte_psel_offset(pf::UARTE_RTS), pin_disconnect_value);
    }

    auto txd = pinctrl::get_pin(pf::UARTE_TXD + group);
    if (txd != -1) {
        gpio_set_option(0, (1 << txd), GPIO_OPT_OUTPUT);
        gpio_set(0, (1 << txd));
        raw_write32(base + uarte_psel_offset(pf::UARTE_TXD), txd);
    } else {
        raw_write32(base + uarte_psel_offset(pf::UARTE_TXD), pin_disconnect_value);
    }

    auto cts = pinctrl::get_pin(pf::UARTE_CTS + group);
    if (cts != -1) {
        gpio_set_option(0, (1 << cts), GPIO_OPT_INPUT);
        raw_write32(base + uarte_psel_offset(pf::UARTE_CTS), cts);
    } else {
        raw_write32(base + uarte_psel_offset(pf::UARTE_CTS), pin_disconnect_value);
    }

    auto rxd = pinctrl::get_pin(pf::UARTE_RXD + group);
    if (rxd != -1) {
        gpio_set_option(0, (1 << rxd), GPIO_OPT_INPUT);
        raw_write32(base + uarte_psel_offset(pf::UARTE_RXD), rxd);
    } else {
        raw_write32(base + uarte_psel_offset(pf::UARTE_RXD), pin_disconnect_value);
    }
}

int _configure_twim(uint32_t base, int group) {
    auto scl = pinctrl::get_pin(pf::TWIM_SCL + group);
    auto sda = pinctrl::get_pin(pf::TWIM_SDA + group);
    if (scl == -1 || sda == -1) {
        return -1;
    }

    raw_write32(base + twim_psel_offset(pf::TWIM_SCL), scl);
    raw_write32(base + twim_psel_offset(pf::TWIM_SDA), sda);

    return 0;
}


}  // namespace

namespace pinctrl {

using pf = function;

int request_function(int pin_function) {
    int pin = get_pin(pin_function);

    int ret = pin;
    switch (pin_function) {
    case pf::UARTE0_GROUP:
        _configure_uarte(uarte0_base, pin_function);
        ret = 0;
        break;
    case pf::UARTE1_GROUP:
        _configure_uarte(uarte1_base, pin_function);
        ret = 0;
        break;
    case pf::TWIM0_GROUP:
        ret = _configure_twim(twim0_base, pin_function);
        break;
    case pf::TWIM1_GROUP:
        ret = _configure_twim(twim1_base, pin_function);
        break;
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
        ret = pin;
        break;
    }

    return ret;
}

}  // namespace pinctrl
