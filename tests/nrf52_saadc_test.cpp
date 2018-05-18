#include "catch.hpp"

#include <cstring>

#include "mock_memio.hpp"

#include "driver/adc.hpp"
#include "nrf52/pinctrl.hpp"

constexpr uint32_t saadc_base = 0x40007000;

// Offsets for various registers
constexpr auto saadc_enable = 0x500;
constexpr auto chx_pselp(unsigned int x) {
    return 0x510 + 16 * x;
}

TEST_CASE("Test ADC API") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    auto get_reg_value = [&mem](uint32_t offset) -> uint32_t {
        return mem.get_value_at(saadc_base + offset);
    };

    SECTION("Test Enable") {
        auto* saadc = driver::ADC::request_by_id(driver::ADC::ID::ADC0);
        REQUIRE(saadc != nullptr);

        CHECK(get_reg_value(saadc_enable) == 1);

        // Based on fake_board configuration
        using ps = pinctrl::saadc;
        CHECK(get_reg_value(chx_pselp(0)) == ps::AIN3);
        CHECK(get_reg_value(chx_pselp(2)) == ps::AIN5);

        for (auto ch : {1, 3, 4, 5, 6, 7}) {
            CAPTURE(ch);
            CHECK(get_reg_value(chx_pselp(ch)) == 0);
        }
    }
}
