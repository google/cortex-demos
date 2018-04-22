#include "driver/timer.hpp"

#include "clk.h"
#include "memio.h"
#include "nrf52/clk.h"

namespace driver {

namespace {

constexpr uint32_t kPeriphBase = 0x40000000;

constexpr uint32_t periph_id_to_base(unsigned int p_id) {
    return kPeriphBase + p_id * 0x1000;
}

class RTC : public Timer {
    public:
        RTC(unsigned int id) : Timer(periph_id_to_base(id), id) {}

        void start() override {
            if (!lfclk_started) {
                clk_request(kLfclkSrc);
                lfclk_started = 1;
            }
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

        void enable_interrupts(uint32_t mask) override {
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
};

bool RTC::lfclk_started = 0;

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
