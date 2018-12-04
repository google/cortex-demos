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

#include <cstdint>

namespace ble {

constexpr unsigned kInterFrameSpaceUs = 150;
constexpr unsigned kCrcPoly = 0x65b;
constexpr uint32_t kAdvAccessAddress = 0x8e89bed6;

class Air {
    public:
        Air() {}

        /** @brief Set BLE Link Layer Channel Index.
         *
         * @param[index] Link Layer Channel Index (0-39).
         *  Note, this is *not* RF Phy Channel.
         */
        virtual int set_channel(unsigned index) = 0;

        /** @brief Set Access Address to be used in
         *      the next transmission/reception.
         *
         *  This address is only for non-advertising packets.
         *  For advertising channel packets the address defined
         *  by BLE standard will be used.
         */
        virtual void set_access_addr(uint32_t addr) = 0;

        static Air* request();
};

}  // namespace ble
