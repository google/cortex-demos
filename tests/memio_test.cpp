/*******************************************************************************
    Copyright 2018 Google LLC

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

#include <cstdint>

#include "memio.h"
#include "mock_memio.hpp"

TEST_CASE("Test Memory IO read/write") {
    constexpr uint32_t test_addr = 0x1000 * 0x1000;
    constexpr uint32_t test_value = 0xdeadbeef;

    auto& mem = mock::get_global_memory();
    mem.reset();

    raw_write32(test_addr, test_value);
    CHECK(raw_read32(test_addr) == test_value);
}
