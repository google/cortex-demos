#pragma once

#include "driver/peripheral.hpp"


namespace driver {

class ADC : public Peripheral {
    public:
        enum class ID {
            ADC0,
            ADC1,
            ADC2,
            ADC3,
            ADC4,
            ADC5,
        };

        ADC(uint32_t base, unsigned int irq_n) : Peripheral(base, irq_n) {}

        virtual int request() { return -1; }
        virtual void start() {}
        virtual void stop() {}

        static ADC* request_by_id(ID id);
};

}  // namespace
