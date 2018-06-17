#include "third_party/catch2/catch.hpp"

#include "mock_memio.hpp"

#include "driver/i2c.hpp"
#include "nrf52/peripheral.hpp"
#include "nrf52/pinctrl.hpp"
#include "pinctrl.hpp"

constexpr uint32_t twim0_base = 0x40003000;
constexpr uint32_t twim1_base = 0x40004000;

constexpr uint32_t twim_base(int index) {
    return index == 0 ? twim0_base : twim1_base;
}

#define REGISTER(name, off) \
    constexpr uint32_t name(int index) {\
        return twim_base(index) + (off);\
    }

REGISTER(psel_scl, 0x508);
REGISTER(psel_sda, 0x50c);

TEST_CASE("Test Request") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    // Self-test
    REQUIRE(psel_scl(0) == (twim0_base + 0x508));
    REQUIRE(psel_scl(1) == (twim1_base + 0x508));

    SECTION("TWIM0") {
        auto* twim = driver::I2C::request_by_id(nrf52::TWIM0);
        REQUIRE(twim != nullptr);

        // Pin number values are taken from nrf52_board_fake
        CHECK(mem.get_value_at(psel_scl(0)) == 15);
        CHECK(mem.get_value_at(psel_sda(0)) == 16);

        // Check that pins are also input and the correct drive
        // strength is set.
    }
}
