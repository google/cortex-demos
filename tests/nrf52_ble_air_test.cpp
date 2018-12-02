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

#include "mock_memio.hpp"

#include "ble/air.hpp"

#include "nrf52/radio.hpp"

using nrf52::Radio;
constexpr uint32_t radio_base = 0x40001000;

TEST_CASE("Phy Test") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    auto get_reg_value = [&mem](uint32_t offset) {
        return mem.get_value_at(radio_base + offset);
    };

    auto* air = ble::Air::request();
    REQUIRE(air != nullptr);

    SECTION("Phy Init Checks") {
        // Check that the mode is set to BLE
        CHECK(get_reg_value(0x510) == Radio::Mode::BLE);
        // TODO: Test CRC Configuration
        // TODO: Test Interframe spacing configuration
        // TODO: Test Center frequency and idle transmission configuration
    }

    SECTION("Test Channel configuration") {

    }
}
