#pragma once

#include <cstdint>

#include "driver/peripheral.hpp"

namespace driver {

class UART : public Peripheral {
    public:
        enum class PARITY {
            NONE,
            EVEN,
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
        };

        UART(uint32_t base, unsigned int irq_n) : Peripheral(base, irq_n) {}

        virtual int request() { return -1; }
        virtual unsigned int set_baudrate(unsigned int) { return 0; }
        virtual int set_parity(PARITY) { return -1; }
        virtual int set_stop_bits(STOP) { return -1; }

        static UART* request_by_id(ID id);
};

}  // namespace driver
