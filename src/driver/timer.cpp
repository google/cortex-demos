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

unsigned int Timer::request_rate(unsigned int req_rate) {
    (void) req_rate;
    return 0;
}

unsigned int Timer::get_base_rate() const {
    return get_rate();
}

}  // namespace driver

namespace arm {

SysTick::SysTick() : driver::Peripheral(kBaseAddr, IRQ_SYSTICK), base_rate_{0} {
    uint32_t calib = raw_read32(kCalibAddr);
    if (calib) {
        auto* rate = (const_cast<unsigned int*>(&base_rate_));
        *rate = 100 * calib;
    }
}

unsigned int SysTick::get_base_rate() const {
    return base_rate_;
}

void SysTick::start() {
    if (base_rate_ == 0) {
        __builtin_trap();
    }
    uint32_t current_value = raw_read32(kCsrAddr);
    raw_write32(kCsrAddr, current_value | kCsrEnable);
}

void SysTick::stop() {
    uint32_t current_value = raw_read32(kCsrAddr);
    raw_write32(kCsrAddr, current_value & ~kCsrEnable);
}

unsigned int SysTick::get_rate() const {
    auto presc = raw_read32(kRvrAddr);

    return presc == 0 ? 0 : (base_rate_ / presc);
}

void SysTick::set_prescaler(unsigned int presc) {
    if (presc < (1 << 24)) {
        raw_write32(kRvrAddr, presc);
    }
}

unsigned int SysTick::request_rate(unsigned int req_rate) {
    if (req_rate > base_rate_) {
        return 0;
    }

    auto presc = base_rate_ / req_rate;
    set_prescaler(presc);
    return base_rate_ / presc;
}

void SysTick::enable_tick_interrupt() {
    uint32_t current_value = raw_read32(kCsrAddr);
    raw_write32(kCsrAddr, current_value | kCsrTickInt);
}

}  // namespace arm


namespace driver {

namespace {

arm::SysTick g_systick;

}  // namespace

Timer* __attribute__((weak)) Timer::request_by_id(Timer::ID id) {
    if (id == Timer::ID::SYSTICK) {
        return &g_systick;
    }

    return nullptr;
}

}  // namespace driver
