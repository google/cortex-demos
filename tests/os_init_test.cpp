/*******************************************************************************
    Copyright 2019 Google LLC

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

#include "core/init.hpp"


TEST_CASE("Test OS initialization") {
    constexpr uint32_t vtor_addr = 0xe000ed08;
    auto& mem = mock::get_global_memory();
    mem.reset();

    CHECK(mem.get_value_at(vtor_addr) == 0);
    CHECK(os::init() >= 0);
    CHECK(mem.get_value_at(vtor_addr) != 0);
    CHECK(nvic_dispatch(IRQ_SVCALL) >= 0);
    CHECK(nvic_dispatch(IRQ_PENDSV) >= 0);
}
