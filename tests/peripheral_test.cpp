#include "catch.hpp"

#include "mock_memio.hpp"

#include "nvic.h"
#include "driver/peripheral.hpp"

namespace {

int counter = 0;

static void dummy_handler() {
    ++counter;
}

}  // namespace

TEST_CASE("Test IRQ Setting") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    nvic_init();
    driver::Peripheral test_periph{0x40001000, 1};

    test_periph.set_irq_handler(dummy_handler);
    CHECK(counter == 0);
    nvic_dispatch(test_periph.get_irq_num());
    CHECK(counter == 1);
}
