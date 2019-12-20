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

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdbool>

#include <vector>

namespace driver {

struct EventInfo {
    int irq_n;
    int evt_id;
    volatile void* src;
    const void* data;
};

class EventHandler {
    public:
        virtual void handle_event(EventInfo* e_info) = 0;
};

class Peripheral {
    public:
        using HandlerContainerT = std::vector<EventHandler*>;
        Peripheral() : base_{0}, irq_n_{0} {};
        Peripheral(uint32_t base, unsigned int irq_n) : base_ {base}, irq_n_{irq_n} {}
        Peripheral(uint32_t base, unsigned int irq_n, HandlerContainerT* evt_handlers);
        Peripheral(const Peripheral&) = delete;

        uint32_t get_base() const {
            return base_;
        }

        unsigned int get_irq_num() const {
            return irq_n_;
        }

        int set_irq_handler(void (*handler)(void)) const;
        void enable_irq() const;
        void disable_irq() const;

        void handle_events();
        int add_event_handler(unsigned int evt, EventHandler* handler);

        virtual void enable_interrupts(uint32_t) {};
        virtual void disable_interrupts(uint32_t) {};

    protected:
        virtual bool is_event_active(int evt) const;
        virtual void clear_event(int evt);

        const uint32_t base_;
        const unsigned int irq_n_;

    private:

        HandlerContainerT* evt_handlers_ = nullptr;
};

}  // namespace driver
