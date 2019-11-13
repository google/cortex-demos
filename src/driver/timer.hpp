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

#pragma once

#include <cstdint>

#include "nvic.h"
#include "driver/peripheral.hpp"

namespace driver {

class Timer : virtual public Peripheral {
    public:
        enum class ID {
            RTC0,
            RTC1,
            RTC2,
            RTC3,
            RTC4,
            RTC5,
            RTC6,

            TIMER0,
            TIMER1,
            TIMER2,
            TIMER3,
            TIMER4,
            TIMER5,
            TIMER6,

            WDT0,
            WDT1,
            WDT2,
            WDT3,
            WDT4,
            WDT5,
            WDT6,

            SYSTICK,
        };

        static Timer* request_by_id(ID id);

        virtual void start() = 0;
        virtual void stop() = 0;
        virtual unsigned int get_rate() const = 0;

        /**
         * @brief Get Timer's base rate in Hz.
         *
         * Presumably, the rate of the timer's input clock.
         * @returns Timer's base rate in Hz.
         */
        virtual unsigned int get_base_rate() const;

        /**
         * @brief Set timer prescaler value.
         *
         * Note, that interpretation of the prescaler value is implementation dependent.
         * In some devices the real prescaler will be 2^presc or something else entirely.
         *
         * @param[presc] New prescaler value.
         */
        virtual void set_prescaler(unsigned int presc) = 0;

        /**
         * @brief Set the timer rate to specified value.
         *
         * Note, that the actual rate may be lower than the requested one,
         * if the timer can't provide such rate, for example.
         * If the device can't provide any rate that is lower than requested one,
         * it will return zero.
         *
         * @param[req_rate] Requested rate in Hz.
         * @returns Actual rate set in Hz, or zero if no acceptable rates can be provided.
         */
        virtual unsigned int request_rate(unsigned int req_rate);

        virtual void enable_tick_interrupt() = 0;
};

}  // namespace driver

/* TODO: Find better location for this driver */
namespace arm {

class SysTick : public driver::Timer {
    public:
        SysTick();
        SysTick(unsigned int base_rate) : driver::Peripheral(kBaseAddr, IRQ_SYSTICK), base_rate_{base_rate} {}

        unsigned int get_base_rate() const override;
        void start() override;
        void stop() override;
        unsigned int get_rate() const override;
        void set_prescaler(unsigned int presc) override;
        unsigned int request_rate(unsigned int req_rate) override;
        void enable_tick_interrupt() override;

    private:
        const unsigned int base_rate_;

        static constexpr uint32_t kBaseAddr = 0xe000'e010;
        static constexpr uint32_t kCsrAddr = kBaseAddr;
        static constexpr uint32_t kRvrAddr = kBaseAddr + 4;
        static constexpr uint32_t kCvrAddr = kBaseAddr + 8;
        static constexpr uint32_t kCalibAddr = kBaseAddr + 12;

        static constexpr uint32_t kCsrCountFlag = (1 << 16);
        static constexpr uint32_t kCsrClkSrc = (1 << 2);
        static constexpr uint32_t kCsrTickInt = (1 << 1);
        static constexpr uint32_t kCsrEnable = (1 << 0);
};

}  // namespace arm
