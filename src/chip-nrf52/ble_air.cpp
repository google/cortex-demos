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

#include "ble/air.hpp"
#include "nrf52/radio.hpp"

namespace nrf52 {

class BleAir : public ble::Air {
    public:
        BleAir() {}

        void set_radio(Radio* radio) {
            radio_ = radio;
        }

        void init_ble() {
            radio_->set_mode(Radio::Mode::BLE);
            radio_->set_ifs(ble::kInterFrameSpaceUs);
            radio_->set_whitening(true);
            radio_->configure_crc(3, true, ble::kCrcPoly);
            radio_->set_base_addr_len(3);
            radio_->set_addr_base(0, ble::kAdvAccessAddress << 8);
            radio_->set_addr_prefix(0, ble::kAdvAccessAddress >> 24);
            // Note: This configuration only supports Uncoded PHY,
            // which is the only thing supported by nRF52832.
            radio_->configure_packet(8, 1, 0);
        }

        int set_channel(unsigned index) override {
            if (index >= kNumRFChannels) {
                return -1;
            }

            auto freq_mhz = 2400;
            if (index < 11) {
                freq_mhz += 4 + index * 2;
            } else if (index < 37) {
                freq_mhz += 6 + index * 2;
            } else if (index == 37) {
                freq_mhz += 2;
            } else if (index == 38) {
                freq_mhz += 26;
            } else {
                freq_mhz += 80;
            }

            radio_->set_frequency(freq_mhz);
            radio_->set_white_iv((1 << 6) | index);

            return 0;
        }

        void set_access_addr(uint32_t addr) override {
            radio_->set_addr_base(1, addr << 8);
            radio_->set_addr_prefix(1, addr >> 24);
        }

    private:
        Radio* radio_;

        static constexpr auto kNumRFChannels = 40;
};

}  // namespace nrf52

namespace {

nrf52::BleAir air;

}  // namespace

namespace ble {

Air* Air::request() {
    auto* radio = nrf52::Radio::request();
    if (!radio) {
        return nullptr;
    }

    air.set_radio(radio);
    air.init_ble();
    return &air;
}

}  // namespace ble
