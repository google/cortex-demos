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

#include "driver/peripheral.hpp"

#include <cstdint>

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
        };

        static Timer* request_by_id(ID id);

        virtual void start() = 0;
        virtual void stop() = 0;
        virtual unsigned int get_rate() const = 0;
        virtual void set_prescaler(unsigned int presc) = 0;

        virtual void enable_tick_interrupt() = 0;
};

}  // namespace driver
