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

#pragma once

#include "nrf52/peripheral.hpp"
#include "nrf52/periph_utils.hpp"

namespace nrf52 {

class Radio : public nrf52::Peripheral {
    public:
        enum Mode : unsigned {
            NRF_1MBIT,
            NRF_2MBIT,
            BLE = 3,
        };

        Radio() : driver::Peripheral(periph::id_to_base(1), 1) {}

        int set_frequency(unsigned int freq_mhz);

        int set_txpower(int8_t txpower);

        void set_mode(enum Mode mode);

        int configure_packet(uint8_t length_bits, uint8_t s0_bytes, uint8_t s1_bits);

        int set_maxlen(uint8_t maxlen);

        int set_base_addr_len(uint8_t base_addr_len);

        void set_whitening(bool enable);

        void set_white_iv(uint8_t iv);

        int set_addr_base(int index, uint32_t base_addr);

        int set_addr_prefix(int index, uint8_t prefix);

        int select_tx_addr(int index);

        int enable_rx_addr(int index);

        int configure_crc(unsigned int len, bool skip_addr, uint32_t crc_poly);

        void set_crc_init(uint32_t crc_init);

        void set_ifs(uint8_t ifs_us);

        static Radio* request();

    private:
        // Enable/Disable the peripheral
        void set_power(bool is_on);

        // Note that the actual range of the radio on nRF52 is 2360 - 2500 MHz,
        // but this driver ignores that feature.
        static constexpr unsigned int kMinFreq = 2400;
        static constexpr unsigned int kMaxFreq = 2500;
        static constexpr auto kMaxAddrIndex = 7;

        static constexpr auto kFrequencyOffset = 0x508;
        static constexpr auto kTxPowerOffset = 0x50c;
        static constexpr auto kModeOffset = 0x510;
        static constexpr auto kPcnf0Offset = 0x514;
        static constexpr auto kPcnf1Offset = 0x518;
        static constexpr auto kBase0Offset = 0x51c;
        static constexpr auto kPrefix0Offset = 0x524;
        static constexpr auto kPrefix1Offset = 0x528;
        static constexpr auto kTxAddressOffset = 0x52c;
        static constexpr auto kRxAddressesOffset = 0x530;
        static constexpr auto kCrcCnfOffset = 0x534;
        static constexpr auto kCrcPolyOffset = 0x538;
        static constexpr auto kCrcInitOffset = 0x53c;
        static constexpr auto kTifsOffset = 0x544;
        static constexpr auto kDataWhiteIVOffset = 0x554;


        static constexpr auto kPowerOffset = 0xffc;

};

}  // namespace nrf52
