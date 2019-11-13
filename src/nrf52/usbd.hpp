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

class USBD : public nrf52::Peripheral {
    public:
        enum EventCause {
            ISOOUTCRC = (1 << 0),
            SUSPEND = (1 << 8),
            RESUME = (1 << 9),
            READY = (1 << 11),

            MASK_ALL = (1 << 0) | (1 << 8) | (1 << 9) | (1 << 11),
        };

        USBD() : driver::Peripheral(periph::id_to_base(39), 39) {}

        /**
         * @brief Get Device's USB Address.
         *
         * USB Address is read only and is set automatically by
         * the hardware during enumeration.
         */
        uint8_t get_addr() const;

        /**
         * @brief Enable/Disable 1.5kOhm pullup on D+.
         *
         * Enabling pullup signals presense of the device to the host.
         * Disabling can be used to simulate physical disconnection
         * of the device from software.
         */
        void pullup(bool enable);

        /**
         * @brief Get the cause of USBEVENT event.
         *
         * @param[mask] Only return (and possibly clear) events in the mask. Default: MASK_ALL
         * @param[clear] Clear the returned events in the register. Default: true.
         *
         * @returns event flags
         */
        uint32_t get_event_cause(enum EventCause mask=MASK_ALL, bool clear=true);


    private:
        static constexpr auto kEventCauseOffset = 0x400;
        static constexpr auto kAddrOffset = 0x470;
        static constexpr auto kPullupOffset = 0x504;
};

}  // namespace nrf52
