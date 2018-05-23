#pragma once

#include "memio.h"
#include "driver/peripheral.hpp"

namespace nrf52 {

class Peripheral : virtual public driver::Peripheral {
    public:
        Peripheral() {};
        void trigger_task(int task) {
            raw_write32(base_ + task * 4, 1);
        }

        void busy_wait_and_clear_event(int evt) {
            while (!raw_read32(base_ + kEventsOffset + evt * 4));
            clear_event(evt);
        }

    private:
        void clear_event(int evt) override {
            raw_write32(base_ + kEventsOffset + evt * 4, 0);
        }

        bool is_event_active(int evt) override {
            return raw_read32(base_ + kEventsOffset + evt * 4);
        }

        static constexpr auto kEventsOffset = 0x100;
};

}  // namespace nrf52
