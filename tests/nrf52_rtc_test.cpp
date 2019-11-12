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

constexpr auto clock_base = 0x40000000;

class DummyEventHandler : public driver::EventHandler {
    public:
        void handle_event(driver::EventInfo* e_info) override {
            (void)e_info;
            ++evt_counter;
        }

        int evt_counter = 0;
};

}  // namespace

TEST_CASE("RTC API") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    const auto& test_start_stop = [&mem](driver::Timer * timer, uint32_t base) {
        // Make LFCLK started event read as 1
        mem.set_value_at(clock_base + 0x104, 1);
        timer->start();
        CHECK(mem.get_value_at(base, 0) == 1);

        timer->stop();
        CHECK(mem.get_value_at(base + 4, 0) == 1);
        // TODO: Test that LFCLK was started only once
        // This seems to be the case based on failures (ironically),
        // but needs to have a reliable test.
    };

    const auto& test_prescaler = [&mem](driver::Timer * timer, uint32_t base) {
        constexpr unsigned int base_rate = 0x8000;
        CHECK(timer->get_rate() == base_rate);

        mem.set_value_at(base + 0x508, 3);
        CHECK(timer->get_rate() == (base_rate / 4));
        CHECK(timer->get_base_rate() == base_rate);

        timer->set_prescaler(8);
        CHECK(timer->get_rate() == (base_rate / 8));
        CHECK(timer->get_base_rate() == base_rate);
    };

    const auto& test_request_rate = [&mem](driver::Timer * timer, uint32_t base) {
        (void)base;
        constexpr unsigned int base_rate = 0x8000;
        constexpr unsigned int max_rate = 32768;
        constexpr unsigned int min_rate = max_rate / (1 << 12);

        CHECK(timer->request_rate(max_rate) == max_rate);
        CHECK(timer->get_rate() == max_rate);

        CHECK(timer->request_rate(min_rate) == min_rate);
        CHECK(timer->get_rate() == min_rate);
        CHECK(timer->get_base_rate() == base_rate);

        auto new_rate = timer->request_rate(823);
        CHECK(new_rate <= 823);
        CHECK(timer->get_rate() == new_rate);

        new_rate = timer->request_rate(200);
        CHECK(new_rate <= 200);
        CHECK(timer->get_rate() == new_rate);
        CHECK(timer->get_base_rate() == base_rate);

        CHECK(timer->request_rate(min_rate - 1) == 0);
        CHECK(timer->request_rate(50000) == max_rate);
        CHECK(timer->get_rate() == max_rate);
    };

    const auto& test_enable_interrupt = [&mem](driver::Timer * timer, uint32_t base) {
        DummyEventHandler dummy_evt_handler;
        timer->add_event_handler(0, &dummy_evt_handler);
        mem.set_value_at(base + 0x100, 1);
        timer->enable_tick_interrupt();

        CHECK((mem.get_value_at(base + 0x304, 0) & 1) == 1);
        nvic_dispatch(timer->get_irq_num());
        CHECK(dummy_evt_handler.evt_counter == 1);
    };

    mem.set_value_at(clock_base + 0x418, (1 << 16) | 1);

    SECTION("RTC0") {
        constexpr auto rtc0_base = 0x4000b000;
        auto* rtc0 = driver::Timer::request_by_id(driver::Timer::ID::RTC0);
        REQUIRE(rtc0 != nullptr);
        CHECK(rtc0->get_irq_num() == 11);

        test_start_stop(rtc0, rtc0_base);
        test_prescaler(rtc0, rtc0_base);
        test_enable_interrupt(rtc0, rtc0_base);
        test_request_rate(rtc0, rtc0_base);
    }

    SECTION("RTC1") {
        constexpr auto rtc1_base = 0x40011000;
        auto* rtc1 = driver::Timer::request_by_id(driver::Timer::ID::RTC1);
        REQUIRE(rtc1 != nullptr);
        CHECK(rtc1->get_irq_num() == 17);

        test_start_stop(rtc1, rtc1_base);
        test_prescaler(rtc1, rtc1_base);
        test_enable_interrupt(rtc1, rtc1_base);
        test_request_rate(rtc1, rtc1_base);
    }

    SECTION("RTC2") {
        constexpr auto rtc2_base = 0x40024000;
        auto* rtc2 = driver::Timer::request_by_id(driver::Timer::ID::RTC2);
        REQUIRE(rtc2 != nullptr);
        CHECK(rtc2->get_irq_num() == 36);

        test_start_stop(rtc2, rtc2_base);
        test_prescaler(rtc2, rtc2_base);
        test_enable_interrupt(rtc2, rtc2_base);
        test_request_rate(rtc2, rtc2_base);
    }
}
