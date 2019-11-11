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

#include "mock_memio.hpp"

#include "driver/timer.hpp"

constexpr uint32_t wdt_base = 0x400e1450;
constexpr uint32_t wdt_cr = wdt_base;
constexpr uint32_t wdt_mr = wdt_base + 4;
constexpr uint32_t wdt_sr = wdt_base + 8;

TEST_CASE("Test Watchdog") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    mem.set_value_at(wdt_mr, 0x3fff'2fff);

    auto* wdt = driver::Timer::request_by_id(driver::Timer::ID::WDT0);
    REQUIRE(wdt);

    CHECK(wdt->get_irq_num() == 4);

    wdt->stop();
    auto mr_value = mem.get_value_at(wdt_mr);
    CHECK((mr_value & (1 << 15)));
}
