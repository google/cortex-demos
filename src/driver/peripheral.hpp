#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdbool>

typedef void (*evt_handler_func_t)(int);

namespace driver {

class Peripheral {
    public:
        Peripheral(uint32_t base, unsigned int irq_n) : base_ {base}, irq_n_{irq_n} {}
        Peripheral(uint32_t base, unsigned int irq_n, evt_handler_func_t* evt_handlers, size_t num_handlers);
        Peripheral(const Peripheral&) = delete;

        uint32_t get_base() const {
            return base_;
        }

        unsigned int get_irq_num() const {
            return irq_n_;
        }

        void set_irq_handler(void (*handler)(void)) const;
        void enable_irq() const;
        void disable_irq() const;

        void handle_events();
        int add_event_handler(unsigned int evt, evt_handler_func_t handler);

        virtual void enable_interrupts(uint32_t mask) = 0;
        virtual void disable_interrupts(uint32_t mask) = 0;

    protected:
        const uint32_t base_;
        const unsigned int irq_n_;

    private:
        virtual bool is_event_active(int evt);
        virtual void clear_event(int evt);
        evt_handler_func_t* evt_handlers_ = nullptr;
        const size_t num_event_handlers_ = 0;
};

}  // namespace driver
