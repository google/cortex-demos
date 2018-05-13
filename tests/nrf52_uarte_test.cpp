#include "catch.hpp"

#include "driver/uarte.hpp"
#include "mock_memio.hpp"

constexpr uint32_t uarte0_base = 0x40002000;
constexpr uint32_t uarte1_base = 0x40028000;

TEST_CASE("TEST UARTE API") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    SECTION("UARTE0") {
        auto* uarte0 = driver::UART::request_by_id(driver::UART::ID::UARTE0);
        REQUIRE(uarte0 != nullptr);

        // Check that the peripheral was enabled
        CHECK(mem.get_value_at(uarte0_base + 0x500) == 8);
    }

    SECTION("UARTE1") {
        auto* uarte1 = driver::UART::request_by_id(driver::UART::ID::UARTE1);
        REQUIRE(uarte1 != nullptr);
    }
}
