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

#include "nvic.h"
#include "driver/timer.hpp"

namespace {

const uint32_t timer_base[] = {
    0x40008000,
    0x40009000,
    0x4000a000,
    0x4001a000,
    0x4001b000,
};

const int timer_irq[] = {8, 9, 10, 26, 27};

const driver::Timer::ID timer_id[] = {
    driver::Timer::ID::TIMER0,
    driver::Timer::ID::TIMER1,
    driver::Timer::ID::TIMER2,
    driver::Timer::ID::TIMER3,
    driver::Timer::ID::TIMER4,
};

}  // namespace

TEST_CASE("Timer API") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    SECTION("Basic API") {
        for (unsigned timer_index = 0; timer_index < 5; ++timer_index) {
            auto* timer = driver::Timer::request_by_id(timer_id[timer_index]);
            REQUIRE(timer != nullptr);
            CHECK(timer->get_irq_num() == timer_irq[timer_index]);

            const auto ct_base = timer_base[timer_index];
            timer->start();
            CHECK(mem.get_value_at(ct_base, 0) == 1);

            timer->stop();
            CHECK(mem.get_value_at(ct_base + 4, 0) == 1);

            timer->set_prescaler(3);
            CHECK(mem.get_value_at(ct_base + 0x510) == 3);

            timer->set_prescaler(9);
            CHECK(mem.get_value_at(ct_base + 0x510) == 9);

            timer->set_prescaler(10);
            CHECK(mem.get_value_at(ct_base + 0x510) == 9);

            timer->set_prescaler(235);
            CHECK(mem.get_value_at(ct_base + 0x510) == 9);

            constexpr unsigned int base_rate = 16'000'000;
            for (unsigned presc = 0; presc < 10; ++presc) {
                mem.set_value_at(ct_base + 0x510, presc);
                auto rate = timer->get_rate();
                auto expected_rate = 16 * 1000 * 1000 / (1 << presc);
                CHECK(rate == expected_rate);
                CHECK(timer->get_base_rate() == base_rate);
            }

            const unsigned int rate_req = 1000'000;
            auto new_rate = timer->request_rate(rate_req);
            CHECK(new_rate > 0);
            CHECK(new_rate == rate_req);

            CHECK(mem.get_value_at(ct_base + 0x510) == 4);

            // Request rate smaller than the minimum one
            new_rate = timer->request_rate(25'000);
            CHECK(new_rate == 0);
            CHECK(timer->get_base_rate() == base_rate);

            // Request largest possible rate
            new_rate = timer->request_rate(16'000'000);
            CHECK(new_rate == 16'000'000);

            // Request even larger rate
            new_rate = timer->request_rate(23'000'000);
            CHECK(new_rate == 16'000'000);
        }
    }
}
