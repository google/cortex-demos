/*******************************************************************************
    Copyright 2018,2019 Google LLC

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

#include "driver/peripheral.hpp"

#include "nvic.h"

namespace driver {

int Peripheral::set_irq_handler(void (*handler)(void)) const {
    return nvic_set_handler(irq_n_, handler);
}

void Peripheral::enable_irq() const {
    nvic_enable_irq(irq_n_);
}

void Peripheral::disable_irq() const {
    nvic_disable_irq(irq_n_);
}

bool Peripheral::is_event_active(int) const {
    return false;
}

Peripheral::Peripheral(uint32_t base, unsigned int irq_n, HandlerContainerT* evt_handlers) :
    base_{base}, irq_n_{irq_n}, evt_handlers_{evt_handlers} {
}

void Peripheral::handle_events() {
    if (!evt_handlers_) {
        return;
    }

    struct EventInfo e_info;
    for (size_t i = 0; i < evt_handlers_->size(); ++i) {
        if ((*evt_handlers_)[i] && is_event_active(i)) {
            e_info.irq_n = irq_n_;
            e_info.evt_id = i;
            (*evt_handlers_)[i]->handle_event(&e_info);
            clear_event(i);
        }
    }
}

int Peripheral::add_event_handler(unsigned int evt, EventHandler* handler) {
    if (evt >= evt_handlers_->size()) {
        return -1;
    }

    (*evt_handlers_)[evt] = handler;
    return 0;
}

void Peripheral::clear_event(int evt) {
    (void)evt;
}

}  // namespace driver
