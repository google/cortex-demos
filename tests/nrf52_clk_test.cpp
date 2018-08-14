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

#include "catch.hpp"

#include "mock_memio.hpp"

#include "clk.h"
#include "nrf52/clk.h"

constexpr auto task_hfclkstart = 0;
constexpr auto task_lfclkstart = 0x8;

TEST_CASE("Test LFCLK Request") {
    constexpr auto clock_base = 0x40000000;
    constexpr auto lfclk_src = 0x518;
    auto& mem = mock::get_global_memory();

    mem.reset();

    mem.set_value_at(clock_base + 0x418, (1 << 16) | 1);
    REQUIRE(clk_request(NRF52_LFCLK_XTAL) >= 0);

    CHECK(mem.get_value_at(clock_base + task_lfclkstart) == 1);
    CHECK(mem.get_value_at(clock_base + lfclk_src) == 1);
}
