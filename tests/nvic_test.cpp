#include "catch.hpp"

#include "mock_memio.hpp"

#include "nvic.h"

int counter = 0;

static void intr_handler() {
    counter++;
}

TEST_CASE("Test Interrupt Dispatch") {
    nvic_set_handler(IRQ_NMI, intr_handler);

    nvic_dispatch(IRQ_NMI);
    CHECK(counter == 1);
    nvic_dispatch(IRQ_NMI);
    CHECK(counter == 2);
}
