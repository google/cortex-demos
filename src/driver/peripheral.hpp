#pragma once

#include <cstdint>

namespace driver {

class Peripheral {
    public:
        Peripheral(uint32_t base, unsigned int irq_n) : base_ {base}, irq_n_{irq_n} {}
        Peripheral(const Peripheral&) = delete;
        virtual ~Peripheral() {}

        uint32_t get_base() const {
            return base_;
        }

        unsigned int get_irq_num() const {
            return irq_n_;
        }

        void set_irq_handler(void (*handler)(void)) const;
        void enable_irq() const;
        void disable_irq() const;

        virtual void enable_interrupts(uint32_t mask) = 0;
        virtual void disable_interrupts(uint32_t mask) = 0;

    protected:
        const uint32_t base_;
        const unsigned int irq_n_;
};

}  // namespace driver
