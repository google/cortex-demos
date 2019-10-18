#pragma once

#include <cstdint>

#include "driver/peripheral.hpp"

namespace driver {

class I2C : virtual public Peripheral {
    public:
        I2C() {}
        I2C(uint32_t base, unsigned int irq_n) : Peripheral(base, irq_n) {}

        virtual int request() {
            return -1;
        }

        static I2C* request_by_id(int id);
};

}  //namespace driver
