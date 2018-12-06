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

#include <vector>

#include "mock_memio.hpp"

#include "ble/air.hpp"
#include "nrf52/radio.hpp"

using nrf52::Radio;
constexpr uint32_t radio_base = 0x40001000;

namespace {

constexpr uint32_t shift_mask(uint32_t value, unsigned int shift, uint32_t mask) {
    return ((value >> shift) & mask);
}

}  // namespace

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
        // Test preamble size.
        CHECK((get_reg_value(0x514) & (1 << 24)) == 0);

        // Check endianness and that the whitening is enabled.
        constexpr uint32_t ew_mask = (1 << 24) | (1 << 25);
        constexpr uint32_t ew_expected = (1 << 25);
        CHECK((get_reg_value(0x518) & ew_mask) == ew_expected);
        // Test Base addr len
        CHECK(shift_mask(get_reg_value(0x518), 16, 7) == 3);
        // This driver permanently configures Advertising Channel Access Address
        // at index 0 and uses index 1 for all other addresses.
        CHECK(shift_mask(get_reg_value(0x51c), 8, 0xffffff) == (ble::kAdvAccessAddress & 0xffffff));
        CHECK(shift_mask(get_reg_value(0x524), 0, 0xff) == (ble::kAdvAccessAddress >> 24));

        // Test CRC Configuration
        constexpr uint32_t crc_len_mask = 3;
        CHECK((get_reg_value(0x534) & crc_len_mask) == 3);
        // Check that CRC does not include the packet address.
        CHECK((get_reg_value(0x534) & (1 << 8)) > 0);
        // Check CRC polynomial
        CHECK(get_reg_value(0x538) == 0x65b);

        // Test Interframe spacing configuration
        CHECK(get_reg_value(0x544) == 150);

        // Test packet configuration
        CHECK(shift_mask(get_reg_value(0x514), 0, 0xf) == 8);
        CHECK((get_reg_value(0x514) & (1 << 8)) > 0);
        CHECK(shift_mask(get_reg_value(0x514), 16, 0xf) == 0);

        // TODO: Test Center frequency and idle transmission configuration
        // TODO: Check that Fast ramp up is enabled. Not needed for BLE,
        //  but maybe more efficient for nrf52.


    }

    SECTION("Test Channel configuration") {
        CHECK(air->set_channel(40) < 0);
        const std::vector<unsigned> channel_freq {
            4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24,
            28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50,
            52, 54, 56, 58, 60, 62, 64, 66, 68, 70,
            72, 74, 76, 78, 2, 26, 80
        };

        REQUIRE(channel_freq.size() == 40);

        for (unsigned i = 0; i < channel_freq.size(); ++i) {
            CHECK(air->set_channel(i) >= 0);
            CHECK(get_reg_value(0x508) == channel_freq[i]);
            // Check Data Whitening Polynomial Configuration
            CHECK(get_reg_value(0x554) == ((1 << 6) | i));
        }
    }

    SECTION("Test Address Setting") {
        constexpr uint32_t test_addr = 0xdead1254;
        air->set_access_addr(test_addr);

        CHECK(shift_mask(get_reg_value(0x520), 8, 0xffffff) == (test_addr & 0xffffff));
        CHECK(shift_mask(get_reg_value(0x524), 8, 0xff) == (test_addr >> 24));

        // Check that we didn't screw up the Advertising address.
        CHECK(shift_mask(get_reg_value(0x51c), 8, 0xffffff) == (ble::kAdvAccessAddress & 0xffffff));
        CHECK(shift_mask(get_reg_value(0x524), 0, 0xff) == (ble::kAdvAccessAddress >> 24));
    }
}
