#include "catch.hpp"

#include "mock_memio.hpp"

#include "gpio.h"

constexpr uint32_t gpio_base = 0x5000 * 0x10000;
constexpr uint32_t out = gpio_base + 0x504;
constexpr uint32_t out_set = gpio_base + 0x508;
constexpr uint32_t out_clr = gpio_base + 0x50c;

TEST_CASE("Verify GPIO set/get memory writes") {
    auto& mem = mock::get_global_memory();

    mem.reset();

    constexpr uint32_t gpio1 = (1 << 12);
    constexpr uint32_t gpio2 = (1 << 14);

    mock::RegSetClearStub out_stub{out, out_set, out_clr};
    mem.set_addr_io_handler(out, out_clr + 4, &out_stub);

    gpio_set(0, gpio1);
    CHECK(mem.get_value_at(out) == gpio1);
    gpio_set(0, gpio2);
    CHECK(mem.get_value_at(out) == (gpio1 | gpio2));

    gpio_clear(0, gpio1);
    CHECK(mem.get_value_at(out) == gpio2);

    gpio_toggle(0, gpio1);
    CHECK(mem.get_value_at(out) == (gpio1 | gpio2));

    gpio_toggle(0, gpio2);
    CHECK(mem.get_value_at(out) == gpio1);
}
