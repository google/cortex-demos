#include "driver/peripheral.hpp"

#include "nvic.h"

namespace driver {

void Peripheral::set_irq_handler(void (*handler)(void)) const {
    nvic_set_handler(irq_n_, handler);
}

void Peripheral::enable_irq() const {
    nvic_enable_irq(irq_n_);
}

void Peripheral::disable_irq() const {
    nvic_disable_irq(irq_n_);
}

}  // namespace driver
