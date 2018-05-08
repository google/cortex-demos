#include "catch.hpp"

#include "driver/uarte.hpp"

constexpr uint32_t uarte0_base = 0x40002000;
constexpr uint32_t uarte1_base = 0x40028000;

TEST_CASE("TEST UARTE API") {
    SECTION("UARTE0") {
        auto* uarte0 = driver::UART::request_by_id(driver::UART::ID::UARTE0);
        REQUIRE(uarte0 != nullptr);
    }
}
