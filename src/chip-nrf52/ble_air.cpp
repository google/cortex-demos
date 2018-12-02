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
        }

    private:
        Radio* radio_;
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
