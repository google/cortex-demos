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

#pragma once

#include "nrf52/peripheral.hpp"
#include "nrf52/periph_utils.hpp"

namespace nrf52 {

// nRF52 specific Power peripheral

class Power : public nrf52::Peripheral {
    public:
        Power();

        bool is_usb_detected() const;
        bool is_usb_power_ready() const;

        static Power* request();

        enum Event {
            POFWARN = 2,
            SLEEPENTER = 5,
            SLEEPEXIT,
            USBDETECTED,
            USBREMOVED,
            USBPWRRDY,

            NUM_EVENTS,
        };

    private:
        static void handle_irq();

        bool is_initialized = false;

        HandlerContainerT event_handlers_{Event::NUM_EVENTS, nullptr};

        static constexpr uint32_t kUSBRegStatusOffset = 0x438;
        static constexpr uint32_t kUSBRegStatusVbusDetect = 1;
        static constexpr uint32_t kUSBRegStatusOutputRdy = 2;
};

}  // namespace nrf52
