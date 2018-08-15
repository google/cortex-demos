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

#include "mock_memio.hpp"

#include "gpio.h"

constexpr uint32_t gpio_base = 0x5000 * 0x10000;
constexpr uint32_t out = gpio_base + 0x504;
constexpr uint32_t out_set = gpio_base + 0x508;
constexpr uint32_t out_clr = gpio_base + 0x50c;
constexpr uint32_t gpio_in = gpio_base + 0x510;
constexpr uint32_t dir = gpio_base + 0x514;
constexpr uint32_t dir_set = gpio_base + 0x518;
constexpr uint32_t dir_clr = gpio_base + 0x51c;

TEST_CASE("Verify GPIO set/get memory writes") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    constexpr uint32_t gpio1 = (1 << 12);
    constexpr uint32_t gpio2 = (1 << 14);

    mock::RegSetClearStub out_stub{out, out_set, out_clr};
    mem.set_addr_io_handler(out, out_clr + 4, &out_stub);

    gpio_set(0, gpio1);
    CHECK(mem.get_value_at(out) == gpio1);
    gpio_set(0, gpio2);
    CHECK(mem.get_value_at(out) == (gpio1 | gpio2));

    gpio_clear(0, gpio1);
    CHECK(mem.get_value_at(out) == gpio2);

    gpio_toggle(0, gpio1);
    CHECK(mem.get_value_at(out) == (gpio1 | gpio2));

    gpio_toggle(0, gpio2);
    CHECK(mem.get_value_at(out) == gpio1);

    mem.set_value_at(gpio_in, gpio1);
    CHECK(gpio_get(0) == gpio1);
}

TEST_CASE("Test various GPIO options") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    mock::RegSetClearStub dir_stub{dir, dir_set, dir_clr};
    mem.set_addr_io_handler(dir, dir_clr + 4, &dir_stub);

    constexpr uint32_t gpio1 = (1 << 7);
    constexpr uint32_t gpio2 = (1 << 24);
    constexpr uint32_t gpio3 = (1 << 30);

    CHECK(gpio_set_option(0, gpio1, GPIO_OPT_OUTPUT) >= 0);
    CHECK((mem.get_value_at(dir) & gpio1) == gpio1);

    CHECK(gpio_set_option(0, gpio2 | gpio3, GPIO_OPT_OUTPUT) >= 0);
    CHECK((mem.get_value_at(dir) & gpio2) == gpio2);
    CHECK((mem.get_value_at(dir) & gpio3) == gpio3);

    CHECK(gpio_set_option(0, gpio2, GPIO_OPT_INPUT) >= 0);
    CHECK((mem.get_value_at(dir) & gpio2) == 0);
}
