/*******************************************************************************
    Copyright 2018,2019 Google LLC

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

#include "clk.h"
#include "nrf52/clk.h"

constexpr auto clock_base = 0x40000000;
constexpr auto lfclk_src = clock_base + 0x518;
constexpr auto hfclkstat = clock_base + 0x40c;
constexpr auto task_hfclkstart = 0;
constexpr auto task_lfclkstart = 0x8;


TEST_CASE("Test LFCLK Request") {
    auto& mem = mock::get_global_memory();

    mem.reset();

    mem.set_value_at(clock_base + 0x104, 1);
    mem.set_value_at(clock_base + 0x418, (1 << 16) | 1);
    REQUIRE(clk_request(NRF52_LFCLK_XTAL) >= 0);

    CHECK(mem.get_op_count(mock::Memory::Op::WRITE32, clock_base + task_lfclkstart) == 1);
    CHECK(mem.get_value_at(lfclk_src) == 1);
    CHECK(mem.get_value_at(clock_base + 0x104) == 0);
}


TEST_CASE("Test HFCLK Request") {
    auto& mem = mock::get_global_memory();

    mem.reset();

    SECTION("Start Xtal When RC is used") {
        // Make HFCLKSTARTED event read as 1
        mem.set_value_at(clock_base + 0x100, 1);
        REQUIRE(clk_request(NRF52_HFCLK_XTAL) >= 0);
        CHECK(mem.get_value_at(clock_base + task_hfclkstart) == 1);
        CHECK(mem.get_op_count(mock::Memory::Op::READ32, clock_base + 0x100) == 1);
        CHECK(mem.get_value_at(clock_base + 0x100) == 0);
    }

    SECTION("Start Xtal, when it's already running") {
        // Make HFCLKSTARTED event read as 1, so that test failure does not hang
        mem.set_value_at(clock_base + 0x100, 1);
        mem.set_value_at(hfclkstat, (1 << 16) | 1);
        REQUIRE(clk_request(NRF52_HFCLK_XTAL) >= 0);
        // Task should not be triggered
        CHECK(mem.get_value_at(clock_base + task_hfclkstart) == 0);
    }
}
