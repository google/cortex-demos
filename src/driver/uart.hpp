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

#include "driver/peripheral.hpp"

namespace driver {

class UART : virtual public Peripheral {
    public:
        enum class PARITY {
            NONE,
            EVEN,
            ODD,
        };

        enum class STOP {
            ONE,
            TWO,
        };

        enum class ID {
            UARTE0,
            UARTE1,
            UARTE2,
            UARTE3,

            UART0,
            UART1,
            UART2,
            UART3,
            UART4,

            USART0,
            USART1,
            USART2,
            USART3,
            USART4,
        };

        UART() {}
        UART(uint32_t base, unsigned int irq_n) : Peripheral(base, irq_n) {}

        virtual int request() {
            return -1;
        }
        virtual unsigned int set_baudrate(unsigned int) {
            return 0;
        }
        virtual int set_parity(PARITY) {
            return -1;
        }
        virtual int set_stop_bits(STOP) {
            return -1;
        }
        virtual size_t write_str(const char*) {
            return 0;
        }

        static UART* request_by_id(ID id);
};

}  // namespace driver
