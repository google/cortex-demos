#pragma once

#include <cstdint>

namespace driver {

class Peripheral {
    public:
        Peripheral(uint32_t base, unsigned int irq_n) : base_ {base}, irq_n_{irq_n} {}
        Peripheral(const Peripheral&) = delete;
        virtual ~Peripheral() {}

    protected:
        const uint32_t base_;
        const unsigned int irq_n_;
};

}  // namespace driver
