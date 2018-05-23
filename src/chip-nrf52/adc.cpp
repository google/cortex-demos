#include "driver/adc.hpp"

#include "cutils.h"
#include "memio.h"
#include "pinctrl.hpp"

#include "nrf52/periph_utils.hpp"
#include "nrf52/pinctrl.hpp"

namespace driver {

namespace {

constexpr auto kSaadcID = 7;

class SAADC : public ADC {
    public:
        SAADC(unsigned int id) : ADC(periph::id_to_base(id), id) {}

        int request() override {
            if (configured_) {
                return 0;
            }

            // Configure Pins
            // Configure Channels

            using pf = pinctrl::function;
            // FIXME: This makes SAADC too intimately familiar with pins configuration, defeating the purpose of
            // pinctrl existance.
            for (unsigned int func = pf::SAADC_CHAN0_GROUP, chan = 0; func <= pf::SAADC_CHAN_LAST_GROUP; func += 3, ++chan) {
                auto pin = pinctrl::request_function(func + pf::SAADC_CHAN_POS);
                if (pin > pinctrl::saadc::NC) {
                    raw_write32(base_ + pselp(chan), pin);
                }

                // TODO: Negative channel
            }

            raw_writeptr(base_ + kResultPtrOffset, result_buffer_);
            raw_write32(base_ + kResultMaxCnt, sizeof(result_buffer_) / 2);

            // Enable Peripheral
            raw_write32(base_ + kEnableOffset, 1);

            configured_ = true;
            return 0;
        }

    private:
        static constexpr auto kEnableOffset = 0x500;
        static constexpr auto kChan0Offset = 0x510;
        static constexpr auto kResultPtrOffset = 0x62c;
        static constexpr auto kResultMaxCnt = 0x630;

        static constexpr uint32_t pselp(unsigned int ch) {
            return kChan0Offset + 16 * ch;
        }

        static constexpr auto kResultBufferLen = 32;
        uint32_t result_buffer_[kResultBufferLen];

        bool configured_ = false;
};

SAADC saadc{kSaadcID};

}  // namespace

ADC* ADC::request_by_id(ADC::ID id) {
    ADC* ret = nullptr;
    switch (id) {
        case ID::ADC0:
            ret = &saadc;
            break;
        default:
            break;
    }

    if (ret) {
        ret->request();
    }

    return ret;
}

}  // namespace driver
