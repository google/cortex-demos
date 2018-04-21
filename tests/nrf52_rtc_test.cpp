#include "catch.hpp"

#include "mock_memio.hpp"

#include "driver/timer.hpp"

TEST_CASE("RTC API") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    constexpr auto rtc1_base = 0x40011000;

    const auto& test_start_stop = [&mem](driver::Timer* timer, uint32_t base) {
        timer->start();
        CHECK(mem.get_value_at(base, 0) == 1);

        timer->stop();
        CHECK(mem.get_value_at(base + 4, 0) == 1);
    };

    SECTION("RTC0") {
        constexpr auto rtc0_base = 0x4000b000;
        auto* rtc0 = driver::Timer::get_by_id(driver::Timer::ID::RTC0);
        CHECK(rtc0 != nullptr);

        test_start_stop(rtc0, rtc0_base);
    }

    SECTION("RTC1") {
        constexpr auto rtc1_base = 0x40011000;
        auto* rtc1 = driver::Timer::get_by_id(driver::Timer::ID::RTC1);
        CHECK(rtc1 != nullptr);

        test_start_stop(rtc1, rtc1_base);
    }
}
