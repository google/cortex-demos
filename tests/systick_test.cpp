/*******************************************************************************
    Copyright 2019 Google LLC

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

#include <cstdint>

#include "mock_memio.hpp"

#include "nvic.h"
#include "driver/timer.hpp"

constexpr uint32_t syst_csr = 0xe000'e010;
constexpr uint32_t syst_rvr = 0xe000'e014;
constexpr uint32_t syst_cvr = 0xe000'e018;
constexpr uint32_t syst_calib = 0xe000'e01c;

TEST_CASE("Test SysTick Timer Initialization") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    auto* systick = driver::Timer::request_by_id(driver::Timer::ID::SYSTICK);
    REQUIRE(systick != nullptr);
}

TEST_CASE("Test Systick API") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    SECTION("Explicit Rate") {
        constexpr unsigned int systick_rate = 64'000'000;
        arm::SysTick systick(systick_rate);
        CHECK(systick.get_base_rate() == systick_rate);
        CHECK(systick.get_irq_num() == IRQ_SYSTICK);

        systick.set_prescaler(64);
        CHECK(systick.get_rate() == systick_rate / 64);
        CHECK(mem.get_value_at(syst_rvr) == 64);

        systick.set_prescaler((1 << 25));
        CHECK(systick.get_rate() == systick_rate / 64);

        CHECK(systick.request_rate(500) == 500);
        CHECK(mem.get_value_at(syst_rvr) == systick_rate / 500);
        CHECK(systick.get_rate() == 500);

        CHECK(systick.request_rate(systick_rate + 1) == 0);

        systick.enable_tick_interrupt();
        CHECK((mem.get_value_at(syst_csr) & (1 << 1)) > 0);

        systick.start();
        CHECK((mem.get_value_at(syst_csr) & (1 << 0)) > 0);

        systick.stop();
        CHECK((mem.get_value_at(syst_csr) & (1 << 0)) == 0);
    }

    SECTION("Calibration Value based SysTick") {
        // Calibration value for 32MHz reference clock.
        mem.set_value_at(syst_calib, 0x4e200);
        arm::SysTick systick;
        CHECK(systick.get_base_rate() == 32'000'000);
    }
}
