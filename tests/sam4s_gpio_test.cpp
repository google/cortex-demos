#include "catch.hpp"

#include <memory>

#include "mock_memio.hpp"

#include "gpio.h"
#include "sam4s/gpio.h"

constexpr uint32_t porta_base = 0x400e0e00;
constexpr uint32_t portb_base = 0x400e1000;
constexpr uint32_t portc_base = 0x400e1200;

constexpr uint32_t out_set(uint32_t base) {
    return base + 0x30;
}

constexpr uint32_t out_clr(uint32_t base) {
    return base + 0x34;
}

constexpr uint32_t out_value(uint32_t base) {
    return base + 0x38;
}

TEST_CASE("Test GPIO set/get/clear memory writes") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    constexpr uint32_t gpio1 = (1 << 12);
    constexpr uint32_t gpio2 = (1 << 14);

    auto port_index = 0;
    for (auto base : {porta_base, portb_base, portc_base}) {
        mock::RegSetClearStub out_stub{out_value(base), out_set(base), out_clr(base)};
        mem.set_addr_io_handler(out_set(base), out_set(base) + 12, &out_stub);

        CAPTURE(port_index);
        CHECK(gpio_set(port_index, gpio1) >= 0);
        CHECK(mem.get_value_at(out_value(base)) == gpio1);

        ++port_index;
    }
}
