#include "driver/timer.hpp"

#include "memio.h"

namespace driver {

namespace {

constexpr uint32_t periph_base = 0x40000000;
constexpr int base_to_irqn(uint32_t base) {
    return (base - periph_base) / 0x1000;
}

constexpr uint32_t periph_id_to_base(unsigned int p_id) {
    return periph_base + p_id * 0x1000;
}

Timer rtc0{periph_id_to_base(11)};
Timer rtc1{periph_id_to_base(17)};
Timer rtc2{periph_id_to_base(36)};

}  // namespace

Timer::Timer(uint32_t base) : base_ {base} {
    irq_n_ = base_to_irqn(base_);
}

void Timer::start() {
    raw_write32(base_, 1);
}

void Timer::stop() {
    raw_write32(base_ + 0x4, 1);
}

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
