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

#include "nrf52/radio.hpp"

constexpr uint32_t radio_base = 0x40001000;

using nrf52::Radio;

namespace {

constexpr uint32_t shift_mask(uint32_t value, unsigned int shift, uint32_t mask) {
    return ((value >> shift) & mask);
}

}  // namespace

TEST_CASE("Radio API") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    auto* radio = nrf52::Radio::request();
    REQUIRE(radio != nullptr);

    CHECK(radio->get_irq_num() == 1);

    auto get_reg_value = [&mem](uint32_t offset) {
        return mem.get_value_at(radio_base + offset);
    };

    // Check that the power is on
    CHECK(mem.get_value_at(radio_base + 0xffc) > 0);

    SECTION("Frequency Setting") {
        CHECK(radio->set_frequency(2250) < 0);
        CHECK(radio->set_frequency(2456) >= 0);
        CHECK(get_reg_value(0x508) == 56);
        CHECK(radio->set_frequency(2832) < 0);
    }

    SECTION("TX Power Setting") {
        // For my own sanity
        radio->set_txpower(-12);
        CHECK(mem.get_value_at(radio_base + 0x50c) == 0xf4);
        for (int8_t power : {
                 4, 3, 0, -4, -8, -12, -16, -20, -40
             }) {
            CHECK(radio->set_txpower(power) >= 0);
            const auto reg_value = (int8_t)get_reg_value(0x50c);
            CHECK(reg_value == power);
        }
    }

    SECTION("Set Mode, Timing, WhiteIV") {
        radio->set_mode(Radio::Mode::BLE);
        CHECK(get_reg_value(0x510) == 3);

        radio->set_ifs(150);
        CHECK(get_reg_value(0x544) == 150);

        radio->set_white_iv(0x45);
        CHECK(get_reg_value(0x554) == 0x45);
    }

    SECTION("Set Packet Conf") {
        constexpr auto lflen = 8;
        constexpr auto s0len = 1;
        constexpr auto s1len = 0;

        // Make sure the preamble bit is preserved
        mem.set_value_at(radio_base + 0x514, (1 << 24));

        CHECK(radio->configure_packet(lflen, s0len, s1len) >= 0);
        uint32_t pcnf0 = get_reg_value(0x514);
        CHECK(shift_mask(pcnf0, 0, 0xf) == lflen);
        CHECK(shift_mask(pcnf0, 8, 1) == s0len);
        CHECK(shift_mask(pcnf0, 16, 0xf) == s1len);
        CHECK((pcnf0 & (1 << 24)) > 0);

        CHECK(radio->configure_packet(3, 0, 8) >= 0);
        pcnf0 = get_reg_value(0x514);
        CHECK(shift_mask(pcnf0, 0, 0xf) == 3);
        CHECK(shift_mask(pcnf0, 8, 1) == 0);
        CHECK(shift_mask(pcnf0, 16, 0xf) == 8);
        CHECK((pcnf0 & (1 << 24)) > 0);
    }

    SECTION("Test Conf 1") {
        constexpr auto maxlen = 37;
        constexpr auto base_addr_len = 3;
        radio->set_whitening(true);
        CHECK(radio->set_maxlen(maxlen) >= 0);
        CHECK(radio->set_base_addr_len(10) < 0);
        CHECK(radio->set_base_addr_len(base_addr_len) >= 0);

        const auto pcnf1 = get_reg_value(0x518);
        CHECK(shift_mask(pcnf1, 0, 0xff) == maxlen);
        CHECK(shift_mask(pcnf1, 16, 7) == base_addr_len);
        CHECK((pcnf1 & (1 << 25)) > 0);
    }

    SECTION("On-Air Address Configuration") {
        constexpr uint32_t base0 = 0xdeadbeef;
        constexpr uint32_t base1 = 0xcafebad0;
        CHECK(radio->set_addr_base(0, base0) >= 0);
        CHECK(radio->set_addr_base(1, base1) >= 0);
        CHECK(radio->set_addr_base(2, base1) < 0);

        CHECK(get_reg_value(0x51c) == base0);
        CHECK(get_reg_value(0x520) == base1);

        CHECK(radio->set_addr_prefix(8, 0xab) < 0);
        mem.set_value_at(radio_base + 0x524, 0);
        mem.set_value_at(radio_base + 0x528, 0);
        for (int i = 0; i < 4; ++i) {
            uint8_t prefix = i * 37;
            uint8_t mask = 0xff;
            CHECK(radio->set_addr_prefix(i, prefix) >= 0);
            const auto reg_value = get_reg_value(0x524);
            CAPTURE(reg_value);
            CHECK(shift_mask(reg_value, i * 8, mask) == prefix);
        }

        for (int i = 4; i < 8; ++i) {
            uint8_t prefix = i * 43;
            CHECK(radio->set_addr_prefix(i, prefix) >= 0);
            const auto reg_value = get_reg_value(0x528);
            CAPTURE(reg_value);
            CHECK(shift_mask(reg_value, (i - 4) * 8, 0xff) == prefix);
        }

        CHECK(radio->select_tx_addr(3) >= 0);
        CHECK(radio->select_tx_addr(12) < 0);
        CHECK(get_reg_value(0x52c) == 3);

        CHECK(radio->enable_rx_addr(3) >= 0);
        CHECK(radio->enable_rx_addr(14) < 0);
        CHECK(radio->enable_rx_addr(5) >= 0);
        CHECK(get_reg_value(0x530) == ((1 << 3) | (1 << 5)));
    }

    SECTION("CRC Configuration") {
        CHECK(radio->configure_crc(3, true, 0x445566) >= 0);
        CHECK(radio->configure_crc(4, true, 0x445566) < 0);

        CHECK(get_reg_value(0x534) == ((1 << 8) | 3));
        CHECK(get_reg_value(0x538) == 0x445566);

        radio->set_crc_init(0x123456);
        CHECK(get_reg_value(0x53c) == 0x123456);
    }
}
