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

#include "third_party/catch2/catch.hpp"

#include <memory>

#include "mock_memio.hpp"

#include "gpio.h"
#include "sam4s/gpio.h"

constexpr uint32_t porta_base = 0x400e0e00;
constexpr uint32_t portb_base = 0x400e1000;
constexpr uint32_t portc_base = 0x400e1200;

constexpr uint32_t out_set(uint32_t base) {
    return base + 0x30;
}

constexpr uint32_t out_clr(uint32_t base) {
    return base + 0x34;
}

constexpr uint32_t out_value(uint32_t base) {
    return base + 0x38;
}

constexpr uint32_t input_value(uint32_t base) {
    return base + 0x3c;
}

constexpr uint32_t pio_per(uint32_t base) {
    return base;
}

constexpr uint32_t pio_pdr(uint32_t base) {
    return base + 4;
}

constexpr uint32_t pio_psr(uint32_t base) {
    return base + 8;
}

constexpr uint32_t pio_oer(uint32_t base) {
    return base + 0x10;
}

constexpr uint32_t pio_odr(uint32_t base) {
    return base + 0x14;
}

constexpr uint32_t pio_osr(uint32_t base) {
    return base + 0x18;
}

TEST_CASE("Test GPIO set/get/clear memory writes") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    constexpr uint32_t gpio1 = (1 << 12);
    constexpr uint32_t gpio2 = (1 << 14);

    auto port_index = 0;
    for (auto base : {
             porta_base, portb_base, portc_base
         }) {
        mock::RegSetClearStub out_stub{out_value(base), out_set(base), out_clr(base)};
        mem.set_addr_io_handler(out_set(base), out_set(base) + 12, &out_stub);

        CAPTURE(port_index);

        CHECK(gpio_set(port_index, gpio1) >= 0);
        CHECK(mem.get_value_at(out_value(base)) == gpio1);

        mem.set_value_at(out_value(base), gpio1 | gpio2);

        CHECK(gpio_clear(port_index, gpio2) >= 0);
        CHECK(mem.get_value_at(out_value(base)) == gpio1);

        CHECK(gpio_toggle(port_index, gpio2) >= 0);
        CHECK(mem.get_value_at(out_value(base)) == (gpio1 | gpio2));
        CHECK(gpio_toggle(port_index, gpio1) >= 0);
        CHECK(mem.get_value_at(out_value(base)) == gpio2);

        mem.set_value_at(input_value(base), gpio2);
        CHECK(gpio_get(port_index) == gpio2);

        ++port_index;
    }
}

TEST_CASE("Test GPIO Direction setting") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    constexpr uint32_t gpio1 = (1 << 13);
    constexpr uint32_t gpio2 = (1 << 18);

    auto port_index = 0;
    for (auto base : {
             porta_base, portb_base, portc_base
         }) {
        mock::RegSetClearStub pio_pxr_stub{pio_psr(base), pio_per(base), pio_pdr(base)};
        mem.set_addr_io_handler(pio_per(base), pio_per(base) + 12, &pio_pxr_stub);

        mock::RegSetClearStub pio_oxr_stub{pio_osr(base), pio_oer(base), pio_odr(base)};
        mem.set_addr_io_handler(pio_oer(base), pio_oer(base) + 12, &pio_oxr_stub);

        // Bring to a known state
        mem.set_value_at(pio_psr(base), 0);
        mem.set_value_at(pio_osr(base), 0);

        CAPTURE(port_index);
        REQUIRE(gpio_set_option(port_index, gpio1, GPIO_OPT_OUTPUT) >= 0);
        CHECK(mem.get_value_at(pio_psr(base)) == gpio1);
        CHECK(mem.get_value_at(pio_osr(base)) == gpio1);

        REQUIRE(gpio_set_option(port_index, gpio2, GPIO_OPT_INPUT) >= 0);
        CHECK(mem.get_value_at(pio_psr(base)) == (gpio1 | gpio2));
        CHECK(mem.get_value_at(pio_osr(base)) == gpio1);

        REQUIRE(gpio_set_option(port_index, gpio1, GPIO_OPT_INPUT) >= 0);
        CHECK(mem.get_value_at(pio_psr(base)) == (gpio1 | gpio2));
        CHECK(mem.get_value_at(pio_osr(base)) == 0);

        ++port_index;
    }
}
