#include "catch.hpp"

#include "mock_memio.hpp"

#include "pinctrl.hpp"
#include "nrf52/pinctrl.hpp"

constexpr uint32_t gpio_base = 0x5000 * 0x10000;
constexpr uint32_t out = gpio_base + 0x504;
constexpr uint32_t out_set = gpio_base + 0x508;
constexpr uint32_t out_clr = gpio_base + 0x50c;
constexpr uint32_t gpio_in = gpio_base + 0x510;
constexpr uint32_t dir = gpio_base + 0x514;
constexpr uint32_t dir_set = gpio_base + 0x518;
constexpr uint32_t dir_clr = gpio_base + 0x51c;

using pf = pinctrl::function;

TEST_CASE("Test UARTE pins configuration") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    mock::RegSetClearStub dir_stub{dir, dir_set, dir_clr};
    mem.set_addr_io_handler(dir, dir_clr + 8, &dir_stub);

    mock::RegSetClearStub out_stub{out, out_set, out_clr};
    mem.set_addr_io_handler(out, out_clr + 8, &out_stub);

    // Numbers are based on fake_board.cpp
    auto pin = pinctrl::get_pin(pf::UARTE0_RXD);
    CHECK(pin == 13);

    pin = pinctrl::get_pin(pf::UARTE0_CTS);
    CHECK(pin == 12);

    auto f_pin = pinctrl::request_function(pf::UARTE0_RXD);
    CHECK(f_pin == 13);

    // Verify that GPIO was properly configured in terms of direction etc.
    CHECK((mem.get_value_at(dir) & (1 << f_pin)) == 0);

    f_pin = pinctrl::request_function(pf::UARTE0_TXD);
    CHECK(f_pin == 3);
    CHECK((mem.get_value_at(dir) & (1 << f_pin)) == (1 << f_pin));
    CHECK((mem.get_value_at(out) & (1 << f_pin)) == (1 << f_pin));
}