/*******************************************************************************
    Copyright 2018 Google LLC

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

#include "clk.h"
#include "memio.h"
#include "nrf52/clk.h"
#include "nrf52/peripheral.hpp"
#include "nrf52/periph_utils.hpp"

namespace driver {

namespace {

constexpr unsigned int kNumRTCEvents = 6;

void rtc0_irq_handler();

void rtc1_irq_handler();

void rtc2_irq_handler();

class RTC : public Timer, public nrf52::Peripheral {
    public:
        RTC(unsigned int id) : driver::Peripheral(periph::id_to_base(id), id, evt_handler_storage_, kNumRTCEvents) {}

        void start() override {
            if (!lfclk_started) {
                clk_request(kLfclkSrc);
                lfclk_started = 1;
            }
            trigger_task(Task::START);
        }

        void stop() override {
            trigger_task(Task::STOP);
        }

        unsigned int get_rate() const override {
            auto presc = raw_read32(base_ + kPrescalerOffset);
            return kBaseRate / ((presc & 0xfff) + 1);
        }

        void set_prescaler(unsigned int presc) override {
            // TODO: debug assert that the timer is stopped
            raw_write32(base_ + kPrescalerOffset, presc - 1);
        }

        void enable_interrupts(uint32_t mask) override {
            if (!irq_handler_configured_) {
                void (*handler)(void) = nullptr;
                switch (irq_n_) {
                    case 11:
                        handler = rtc0_irq_handler;
                        break;
                    case 17:
                        handler = rtc1_irq_handler;
                        break;
                    case 36:
                        handler = rtc2_irq_handler;
                        break;
                }

                if (handler) {
                    set_irq_handler(handler);
                    irq_handler_configured_ = true;
                }
            }
            raw_write32(base_ + kIntenSetOffset, mask);
            raw_write32(base_ + kEvtenSetOffset, mask);
            enable_irq();
        }

        void disable_interrupts(uint32_t mask) override {
            raw_write32(base_ + kIntenClrOffset, mask);
        }

        void enable_tick_interrupt() override {
            enable_interrupts(kIntenTick);
        }

    private:
        enum Task {
            START,
            STOP
        };

        static constexpr auto kIntenSetOffset = 0x304;
        static constexpr auto kIntenClrOffset = 0x308;
        static constexpr auto kEvtenSetOffset = 0x344;
        static constexpr auto kEvtenClrOffset = 0x348;

        static constexpr auto kPrescalerOffset = 0x508;
        static constexpr auto kBaseRate = 32768;

        static constexpr uint32_t kIntenTick = (1 << 0);

        static bool lfclk_started;
        // TODO: make this configurable
        static constexpr auto kLfclkSrc = NRF52_LFCLK_XTAL;

        evt_handler_func_t evt_handler_storage_[kNumRTCEvents];

        bool irq_handler_configured_ = false;
};

bool RTC::lfclk_started = 0;

RTC rtc0{11};
RTC rtc1{17};
RTC rtc2{36};

void rtc0_irq_handler() {
    rtc0.handle_events();
}

void rtc1_irq_handler() {
    rtc1.handle_events();
}

void rtc2_irq_handler() {
    rtc2.handle_events();
}

}  // namespace

Timer* Timer::get_by_id(ID id) {
    Timer* ret = nullptr;
    switch (id) {
        case ID::RTC0:
            ret = &rtc0;
            break;
        case ID::RTC1:
            ret = &rtc1;
            break;
        case ID::RTC2:
            ret = &rtc2;
            break;
        default:
            break;
    };

    return ret;
}

}  // namespace driver
