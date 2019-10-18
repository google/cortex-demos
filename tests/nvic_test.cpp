/*******************************************************************************
    Copyright 2018,2019 Google LLC

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        https://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*******************************************************************************/

#include "third_party/catch2/catch.hpp"

#include "mock_memio.hpp"

#include "nvic.h"

int counter = 0;

static void intr_handler() {
    counter++;
}

TEST_CASE("Test IRQ attributes setting") {
    constexpr uint32_t nvic_iser = 0xe000e100;
    auto& mem = mock::get_global_memory();
    mem.reset();

    nvic_enable_irq(12);
    CHECK((mem.get_value_at(nvic_iser) & (1 << 12)) > 0);
    nvic_enable_irq(32);
    CHECK((mem.get_value_at(nvic_iser + 4) & (1 << 0)) > 0);
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
    // Check that dispatch of uninitialized interrupt handler
    // does the right thing and does not explode.
    CHECK(nvic_dispatch(0) < 0);

    nvic_set_handler(IRQ_NMI, intr_handler);
    nvic_set_handler(0, intr_handler);

    CHECK(nvic_dispatch(IRQ_NMI) >= 0);
    CHECK(counter == 1);
    CHECK(nvic_dispatch(IRQ_NMI) >= 0);
    CHECK(counter == 2);
    CHECK(nvic_dispatch(0) >= 0);
    CHECK(counter == 3);
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

    SECTION("Test setting off IRQs") {
        constexpr uint32_t nvic_ispr = 0xe000e200;
        nvic_irqset(1);
        CHECK((mem.get_value_at(nvic_ispr) & (1 << 1)) > 0);
    }
}
