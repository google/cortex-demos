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
        USBD() : driver::Peripheral(periph::id_to_base(39), 39) {}

        /**
         * @brief Get Device's USB Address.
         *
         * USB Address is read only and is set automatically by
         * the hardware during enumeration.
         */
        uint8_t get_addr() const;

    private:
        static constexpr auto kAddrOffset = 0x470;
};

}  // namespace nrf52
