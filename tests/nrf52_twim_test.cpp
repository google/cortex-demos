#include "third_party/catch2/catch.hpp"

#include "mock_memio.hpp"

#include "driver/i2c.hpp"
#include "nrf52/peripheral.hpp"
#include "nrf52/pinctrl.hpp"
#include "pinctrl.hpp"

constexpr uint32_t twim0_base = 0x40003000;
constexpr uint32_t twim1_base = 0x40004000;

TEST_CASE("Test Request") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    SECTION("TWIM0") {
        auto* twim = driver::I2C::request_by_id(nrf52::TWIM0);
        REQUIRE(twim != nullptr);
    }
}
