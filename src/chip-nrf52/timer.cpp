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
#include "cutils.h"
#include "memio.h"
#include "nvic.h"
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
        RTC(unsigned int id) : driver::Peripheral(periph::id_to_base(id), id, &evt_handler_storage_) {}
        RTC(unsigned int id, irq_handler_func_t irq_handler) : driver::Peripheral(periph::id_to_base(id), id,
                    &evt_handler_storage_),
            irq_handler_{irq_handler} {}

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

        unsigned int get_base_rate() const override {
            return kBaseRate;
        }

        void set_prescaler(unsigned int presc) override {
            // TODO: debug assert that the timer is stopped
            raw_write32(base_ + kPrescalerOffset, presc - 1);
        }

        unsigned int request_rate(unsigned int req_rate) override {
            constexpr unsigned int min_rate = kBaseRate / (1 << 12);
            // This is the smallest possible rate right now.
            if (req_rate < min_rate) {
                return 0;
            }

            req_rate = MIN(kBaseRate, req_rate);

            unsigned int presc = MIN(kBaseRate / req_rate, kMaxPrescaler);
            unsigned int new_rate = kBaseRate / presc;
            if (new_rate > req_rate) {
                ++presc;
                new_rate = kBaseRate / presc;
            }

            set_prescaler(presc);
            return kBaseRate / presc;
        }

        void enable_interrupts(uint32_t mask) override {
            if (!irq_handler_configured_) {
                if (irq_handler_) {
                    set_irq_handler(irq_handler_);
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
        static constexpr unsigned int kBaseRate = 32768;
        static constexpr unsigned int kMaxPrescaler = (1 << 12);

        static constexpr uint32_t kIntenTick = (1 << 0);

        static bool lfclk_started;
        // TODO: make this configurable
        static constexpr auto kLfclkSrc = NRF52_LFCLK_XTAL;

        HandlerContainerT evt_handler_storage_{kNumRTCEvents, nullptr};

        bool irq_handler_configured_ = false;
        irq_handler_func_t irq_handler_ = nullptr;
};

bool RTC::lfclk_started = 0;

RTC rtc0{11, rtc0_irq_handler};
RTC rtc1{17, rtc1_irq_handler};
RTC rtc2{36, rtc2_irq_handler};

void rtc0_irq_handler() {
    rtc0.handle_events();
}

void rtc1_irq_handler() {
    rtc1.handle_events();
}

void rtc2_irq_handler() {
    rtc2.handle_events();
}

class TimerCounter : public Timer, public nrf52::Peripheral {
    public:
        enum Task {
            START,
            STOP,
            COUNT,
            CLEAR,
            SHUTDOWN,
        };

        TimerCounter(unsigned int id) : driver::Peripheral(periph::id_to_base(id), id, &evt_handler_storage_) {}

        void start() override {
            trigger_task(Task::START);
        }

        void stop() override {
            trigger_task(Task::STOP);
        }

        unsigned int get_rate() const override {
            unsigned int presc = raw_read32(base_ + kPrescalerOffset);

            return kInputRate / (1 << presc);
        }

        unsigned int get_base_rate() const override {
            return kInputRate;
        }

        void set_prescaler(unsigned int presc) override {
            unsigned int real_presc = MIN(presc, kMaxPrescaler);
            raw_write32(base_ + kPrescalerOffset, real_presc);
        }

        unsigned int request_rate(unsigned int req_rate) override {
            unsigned int presc = 10;
            const auto min_rate = kInputRate / (1 << (presc - 1));
            // This is the smallest possible rate right now.
            if (req_rate < min_rate) {
                return 0;
            }

            for (; presc > 0; --presc) {
                if ((kInputRate / (1 << (presc - 1))) > req_rate) {
                    break;
                }
            }

            set_prescaler(presc);
            return kInputRate / (1 << presc);
        }

        void enable_tick_interrupt() override {}

    private:
        static constexpr unsigned kMaxPrescaler = 9;
        static constexpr unsigned int kInputRate = 16 * 1000 * 1000;

        static constexpr uint32_t kPrescalerOffset = 0x510;

        static constexpr unsigned int kNumTimerEvents = 6;
        HandlerContainerT evt_handler_storage_{kNumTimerEvents, nullptr};

};

TimerCounter timer0{8};
TimerCounter timer1{9};
TimerCounter timer2{10};
TimerCounter timer3{26};
TimerCounter timer4{27};

// FIXME: Get the value from the board configuration.
arm::SysTick systick(64'000'000);

}  // namespace

Timer* Timer::request_by_id(ID id) {
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
    case ID::TIMER0:
        ret = &timer0;
        break;
    case ID::TIMER1:
        ret = &timer1;
        break;
    case ID::TIMER2:
        ret = &timer2;
        break;
    case ID::TIMER3:
        ret = &timer3;
        break;
    case ID::TIMER4:
        ret = &timer4;
        break;
    case ID::SYSTICK:
        ret = &systick;
        break;
    default:
        break;
    };

    return ret;
}

}  // namespace driver
