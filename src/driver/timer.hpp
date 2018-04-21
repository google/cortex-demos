#pragma once

#include "driver/peripheral.hpp"

#include <cstdint>

namespace driver {

class Timer : public Peripheral {
    public:
        enum class ID {
            RTC0,
            RTC1,
            RTC2,
            RTC3,
            RTC4,
            RTC5,
            RTC6,
        };

        static Timer* get_by_id(ID id);

        virtual void start() = 0;
        virtual void stop() = 0;
        virtual unsigned int get_rate() const = 0;
        virtual void  set_prescaler(unsigned int presc) = 0;

    protected:
        Timer(uint32_t base, unsigned int irq_n) : Peripheral(base, irq_n) {}
};

}  // namespace driver
