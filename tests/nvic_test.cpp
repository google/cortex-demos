#include "catch.hpp"

#include "mock_memio.hpp"

#include "nvic.h"

int counter = 0;

static void intr_handler() {
    counter++;
}

TEST_CASE("Test NVIC Init") {
    constexpr uint32_t vtor_addr = 0xe000ed08;
    auto& mem = mock::get_global_memory();
    mem.reset();

    CHECK(mem.get_value_at(vtor_addr) == 0);
    nvic_init();
    CHECK(mem.get_value_at(vtor_addr) != 0);
}

TEST_CASE("Test Interrupt Dispatch") {
    nvic_set_handler(IRQ_NMI, intr_handler);

    nvic_dispatch(IRQ_NMI);
    CHECK(counter == 1);
    nvic_dispatch(IRQ_NMI);
    CHECK(counter == 2);
}

TEST_CASE("Test Setting Off Interrupts") {
    const uint32_t icsr_addr = 0xe000ed04;
    auto& mem = mock::get_global_memory();
    mem.reset();

    SECTION("Test Setting Off special interrupts") {
        nvic_irqset(IRQ_NMI);
        CHECK((mem.get_value_at(icsr_addr) & (1 << 31)) > 0);
        nvic_irqset(IRQ_PENDSV);
        CHECK((mem.get_value_at(icsr_addr) & (1 << 28)) > 0);
        nvic_irqset(IRQ_SYSTICK);
        CHECK((mem.get_value_at(icsr_addr) & (1 << 26)) > 0);
    }
}
