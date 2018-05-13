#pragma once

#include <cstdint>

#include "driver/peripheral.hpp"

namespace driver {

class UART : public Peripheral {
    public:
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
        };

        UART(uint32_t base, unsigned int irq_n) : Peripheral(base, irq_n) {}

        virtual int request() { return -1; }

        static UART* request_by_id(ID id);
};

}  // namespace driver
