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


#include "nrf52/power.hpp"

namespace nrf52 {
namespace {

Power power;

}  // namespace

Power::Power() : driver::Peripheral(periph::id_to_base(0), 0, &event_handlers_) {
}

void Power::handle_irq() {
    power.handle_events();
}

Power* Power::request() {
    if (!power.is_initialized) {
        int ret = power.set_irq_handler(Power::handle_irq);
        if (ret < 0) {
            return nullptr;
        }
        power.is_initialized = true;
    }
    return &power;
}

bool Power::is_usb_detected() const {
    const uint32_t usb_ready_value = raw_read32(base_ + kUSBRegStatusOffset);
    return is_event_active(Event::USBDETECTED) ||
           (usb_ready_value & kUSBRegStatusVbusDetect);
}

bool Power::is_usb_power_ready() const {
    const uint32_t usb_ready_value = raw_read32(base_ + kUSBRegStatusOffset);
    return is_event_active(Event::USBPWRRDY) ||
           (usb_ready_value & kUSBRegStatusOutputRdy);
}

}  // namespace nrf52
