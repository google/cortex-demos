#include "driver/timer.hpp"

#include "memio.h"

namespace driver {

namespace {

constexpr uint32_t kPeriphBase = 0x40000000;

constexpr uint32_t periph_id_to_base(unsigned int p_id) {
    return kPeriphBase + p_id * 0x1000;
}

class RTC : public Timer {
    public:
        RTC(unsigned int id) : base_{periph_id_to_base(id)}, irq_n_ {id} {}

        void start() override {
            raw_write32(base_, 1);
        }

        void stop() override {
            raw_write32(base_ + 4, 1);
        }

        unsigned int get_rate() const override {
            auto presc = raw_read32(base_ + kPrescalerOffset);
            return kBaseRate / ((presc & 0xfff) + 1);
        }

        void set_prescaler(unsigned int presc) override {
            // TODO: debug assert that the timer is stopped
            raw_write32(base_ + kPrescalerOffset, presc - 1);
        }

    private:
        static constexpr auto kPrescalerOffset = 0x508;
        static constexpr auto kBaseRate = 32768;

        const uint32_t base_;
        const unsigned int irq_n_;
};

RTC rtc0{11};
RTC rtc1{17};
RTC rtc2{36};

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
