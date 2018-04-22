#include "catch.hpp"

#include <iostream>

#include "mock_memio.hpp"

#include "nvic.h"
#include "driver/timer.hpp"

namespace {

constexpr auto clock_base = 0x40000000;

int counter;

void dummy_handler() {
    ++counter;
}

}  // namespace

TEST_CASE("RTC API") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    const auto& test_start_stop = [&mem](driver::Timer* timer, uint32_t base) {
        timer->start();
        CHECK(mem.get_value_at(base, 0) == 1);

        timer->stop();
        CHECK(mem.get_value_at(base + 4, 0) == 1);
        // TODO: Test that LFCLK was started only once
        // This seems to be the case based on failures (ironically),
        // but needs to have a reliable test.
    };

    const auto& test_prescaler = [&mem](driver::Timer* timer, uint32_t base) {
        constexpr unsigned int base_rate = 0x8000;
        CHECK(timer->get_rate() == base_rate);

        mem.set_value_at(base + 0x508, 3);
        CHECK(timer->get_rate() == (base_rate / 4));

        timer->set_prescaler(8);
        CHECK(timer->get_rate() == (base_rate / 8));
    };

    const auto& test_enable_interrupt = [&mem](driver::Timer* timer, uint32_t base) {
        timer->set_irq_handler(dummy_handler);
        timer->enable_tick_interrupt();

        CHECK((mem.get_value_at(base + 0x304, 0) & 1) == 1);
        nvic_dispatch(timer->get_irq_num());
        CHECK(counter == 1);
    };

    counter = 0;
    mem.set_value_at(clock_base + 0x418, (1 << 16) | 1);

    SECTION("RTC0") {
        constexpr auto rtc0_base = 0x4000b000;
        auto* rtc0 = driver::Timer::get_by_id(driver::Timer::ID::RTC0);
        REQUIRE(rtc0 != nullptr);
        CHECK(rtc0->get_irq_num() == 11);

        test_start_stop(rtc0, rtc0_base);
        test_prescaler(rtc0, rtc0_base);
        test_enable_interrupt(rtc0, rtc0_base);
    }

    SECTION("RTC1") {
        constexpr auto rtc1_base = 0x40011000;
        auto* rtc1 = driver::Timer::get_by_id(driver::Timer::ID::RTC1);
        REQUIRE(rtc1 != nullptr);
        CHECK(rtc1->get_irq_num() == 17);

        test_start_stop(rtc1, rtc1_base);
        test_prescaler(rtc1, rtc1_base);
        test_enable_interrupt(rtc1, rtc1_base);
    }

    SECTION("RTC2") {
        constexpr auto rtc2_base = 0x40024000;
        auto* rtc2 = driver::Timer::get_by_id(driver::Timer::ID::RTC2);
        REQUIRE(rtc2 != nullptr);
        CHECK(rtc2->get_irq_num() == 36);

        test_start_stop(rtc2, rtc2_base);
        test_prescaler(rtc2, rtc2_base);
        test_enable_interrupt(rtc2, rtc2_base);
    }
}
