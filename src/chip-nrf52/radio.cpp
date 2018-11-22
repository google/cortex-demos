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

#include "nrf52/radio.hpp"

#include "memio.h"

namespace nrf52 {

namespace {

Radio radio;

}  // namespace

int Radio::set_frequency(unsigned int freq_mhz) {
    if (freq_mhz < kMinFreq || freq_mhz > kMaxFreq) {
        return -1;
    }

    raw_write32(base_ + kFrequencyOffset, freq_mhz - kMinFreq);

    return 0;
}

void Radio::set_power(bool is_on) {
    raw_write32(base_ + kPowerOffset, is_on ? 1 : 0);
}

int Radio::set_txpower(int8_t txpower) {
    raw_write32(base_ + kTxPowerOffset, txpower & 0xff);
    return 0;
}

void Radio::set_mode(enum Mode mode) {
    raw_write32(base_ + kModeOffset, mode);
}

Radio* Radio::request() {
    radio.set_power(true);
    return &radio;
}

int Radio::configure_packet(uint8_t length_bits, uint8_t s0_bytes, uint8_t s1_bits) {
    if (length_bits > 0xf) {
        return -1;
    }

    if (s0_bytes > 1) {
        return -2;
    }

    if (s1_bits > 0xf) {
        return -3;
    }

    constexpr uint32_t pconf_mask = (0xf | (1 << 8) | (0xf << 16));
    uint32_t value = (length_bits | (s0_bytes << 8) | (s1_bits << 16));
    raw_set_masked(base_ + kPcnf0Offset, pconf_mask, value);
    return 0;
}

int Radio::set_maxlen(uint8_t maxlen) {
    raw_set_masked(base_ + kPcnf1Offset, 0xff, maxlen);
    return 0;
}

int Radio::set_base_addr_len(uint8_t base_addr_len) {
    if (base_addr_len > 4 || base_addr_len < 2) {
        return -1;
    }

    raw_set_masked(base_ + kPcnf1Offset, (7 << 16), (base_addr_len << 16));
    return 0;
}

void Radio::set_whitening(bool enable) {
    if (enable) {
        raw_setbits_le32(base_ + kPcnf1Offset, (1 << 25));
    } else {
        raw_clrbits_le32(base_ + kPcnf1Offset, (1 << 25));
    }
}

int Radio::set_addr_base(int index, uint32_t base_addr) {
    if (index > 1) {
        return -1;
    }

    const uint32_t reg_addr = base_ + kBase0Offset + 4 * index;
    raw_write32(reg_addr, base_addr);
    return 0;
}

int Radio::set_addr_prefix(int index, uint8_t prefix) {
    if (index > kMaxAddrIndex) {
        return -1;
    }

    const uint32_t reg_addr = base_ + (index < 4 ? kPrefix0Offset : kPrefix1Offset);
    const unsigned prefix_shift = (index % 4) * 8;
    raw_set_masked(reg_addr, 0xff << prefix_shift, prefix << prefix_shift);
    return 0;
}

int Radio::select_tx_addr(int index) {
    if (index > kMaxAddrIndex) {
        return -1;
    }

    raw_write32(base_ + kTxAddressOffset, index);

    return 0;
}

int Radio::enable_rx_addr(int index) {
    if (index > kMaxAddrIndex) {
        return -1;
    }

    raw_setbits_le32(base_ + kRxAddressesOffset, (1 << index));

    return 0;
}

int Radio::configure_crc(unsigned int len, bool skip_addr, uint32_t crc_poly) {
    if (len > 3) {
        return -1;
    }

    raw_write32(base_ + kCrcPolyOffset, crc_poly);

    uint32_t conf = len;
    if (skip_addr) {
        conf |= (1 << 8);
    }

    raw_write32(base_ + kCrcCnfOffset, conf);

    return 0;
}

void Radio::set_crc_init(uint32_t crc_init) {
    raw_write32(base_ + kCrcInitOffset, crc_init);
}

void Radio::set_ifs(uint8_t ifs_us) {
    raw_write32(base_ + kTifsOffset, ifs_us);
}

void Radio::set_white_iv(uint8_t iv) {
    raw_write32(base_ + kDataWhiteIVOffset, iv);
}

}  // namespace nrf52
