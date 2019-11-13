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

#include "nrf52/usbd.hpp"


namespace nrf52 {

uint8_t USBD::get_addr() const {
    return raw_read32(base_ + kAddrOffset);
}

void USBD::pullup(bool enable) {
    const uint32_t value = enable ? 1 : 0;
    raw_write32(base_ + kPullupOffset, value);
}

uint32_t USBD::get_event_cause(enum EventCause mask, bool clear) {
    uint32_t value = raw_read32(base_ + kEventCauseOffset);
    value &= mask;

    if (clear && value) {
        raw_write32(base_ + kEventCauseOffset, value);
    }

    return value;
}

}  // namespace nrf52
