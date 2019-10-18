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

#include "driver/adc.hpp"

#include "cutils.h"
#include "memio.h"
#include "pinctrl.hpp"

#include "nrf52/peripheral.hpp"
#include "nrf52/periph_utils.hpp"
#include "nrf52/pinctrl.hpp"

namespace driver {

namespace {

constexpr auto kSaadcID = 7;

class SAADC : public ADC, public nrf52::Peripheral {
    public:
        SAADC(unsigned int id) : driver::Peripheral(periph::id_to_base(id), id) {}

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
                    chan_offset_map_[chan] = num_channels_;
                    ++num_channels_;
                } else {
                    chan_offset_map_[chan] = 0xff;
                }

                // TODO: Negative channel
            }

            raw_writeptr(base_ + kResultPtrOffset, result_buffer_);
            raw_write32(base_ + kResultMaxCnt, sizeof(result_buffer_) * 2);

            // Enable Peripheral
            raw_write32(base_ + kEnableOffset, 1);

            configured_ = true;
            return 0;
        }

        int start(int max_samples) override {
            raw_writeptr(base_ + kResultPtrOffset, result_buffer_);
            trigger_task(Task::START);
            busy_wait_and_clear_event(Event::STARTED);

            auto num_samples = MIN(static_cast<unsigned>(max_samples), (sizeof(result_buffer_) / 2) / num_channels_);
            for (unsigned i = 0; i < num_samples; ++i) {
                trigger_task(Task::SAMPLE);
                busy_wait_and_clear_event(Event::DONE);
            }

            return num_samples;
        }

        unsigned get_num_channels() const override {
            return num_channels_;
        }

        uint32_t get_result(unsigned channel, unsigned sample) override {
            if (channel >= kMaxChannels) {
                return 0;
            }

            if (chan_offset_map_[channel] == 0xff) {
                return 0;
            }

            unsigned result_offset = sample * num_channels_ + chan_offset_map_[channel];
            if (result_offset >= sizeof(result_buffer_) / 2) {
                return 0;
            }

            return reinterpret_cast<uint16_t*>(result_buffer_)[result_offset];
        }

    private:
        enum Task {
            START,
            SAMPLE,
            STOP,
            CALIBRATEOFFSET,
        };

        enum Event {
            STARTED,
            END,
            DONE,
        };

        static constexpr auto kEnableOffset = 0x500;
        static constexpr auto kChan0Offset = 0x510;
        static constexpr auto kResultPtrOffset = 0x62c;
        static constexpr auto kResultMaxCnt = 0x630;

        static constexpr uint32_t pselp(unsigned int ch) {
            return kChan0Offset + 16 * ch;
        }

        static constexpr auto kResultBufferLen = 32;
        uint32_t result_buffer_[kResultBufferLen];

        static constexpr auto kMaxChannels = 8;
        uint8_t chan_offset_map_[kMaxChannels];


        bool configured_ = false;
        unsigned num_channels_ = 0;
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
