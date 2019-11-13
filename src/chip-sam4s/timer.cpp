/*******************************************************************************
    Copyright 2019 Google LLC

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

#include "driver/timer.hpp"

#include "memio.h"

namespace driver {

namespace {

class WDT : public Timer {
    public:
        WDT() : Peripheral(0x400e'1450, 4) {}

        void start() override {

        }

        void stop() override {
            /* Note, that this disables the watchdog
             * and can only be done if the WDT_MR wasn't previously accessed
             */
            uint32_t mr = raw_read32(base_ + kMROffset);
            raw_write32(base_ + kMROffset, mr | kMRWdtDis);
        }

        unsigned int get_rate() const override {
            return 0;
        }

        void set_prescaler(unsigned int presc) override {
            (void)presc;
        }

        void enable_tick_interrupt() override {

        }

    private:
        static constexpr uint32_t kCROffset = 0;
        static constexpr uint32_t kMROffset = 4;
        static constexpr uint32_t kSROffset = 8;

        static constexpr uint32_t kMRWdtDis = (1 << 15);
} wdt0;

// FIXME: SAM4S systick clock source is far more complicated than this...
arm::SysTick systick;

}  // namespace

Timer* Timer::request_by_id(Timer::ID id) {
    if (id == Timer::ID::WDT0) {
        return &wdt0;
    } else if (id == Timer::ID::SYSTICK) {
        return &systick;
    }
    return nullptr;
}

}  // namespace driver
