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

#include "driver/uart.hpp"

#include "cutils.h"
#include "memio.h"
#include "pinctrl.hpp"

#include "nrf52/peripheral.hpp"
#include "nrf52/periph_utils.hpp"
#include "nrf52/pinctrl.hpp"


namespace driver {

namespace {

// Offset for various registers

const struct rate_config {
    unsigned int rate;
    uint32_t conf_value;
    unsigned int actual_rate;
} supported_rates[] = {
    { .rate = 1200, .conf_value = 0x4f000, .actual_rate = 1205 },
    { .rate = 2400, .conf_value = 0x9d000, .actual_rate = 2396 },
    { .rate = 4800, .conf_value = 0x13b000, .actual_rate = 4808 },
    // Skipped many values
    { .rate = 56000, .conf_value = 0xe50000, .actual_rate = 55944 },
    { .rate = 115200, .conf_value = 0x1d60000, .actual_rate = 115108 },
    { .rate = 921600, .conf_value = 0xf000000, .actual_rate = 941176 },
    { .rate = 1000 * 1000, .conf_value = 0x10000000, .actual_rate = 1000 * 1000 },
};

constexpr auto kUarte0ID = 2;
constexpr auto kUarte1ID = 40;

class UARTE : public UART, public nrf52::Peripheral {
    public:
        UARTE(unsigned int id) : driver::Peripheral(periph::id_to_base(id), id) {}

        int request() override {
            if (configured_) {
                return 0;
            }

            raw_write32(base_ + kEnableOffset, kEnableValue);

            using pf = pinctrl::function;
            auto func_group = base_ == periph::id_to_base(kUarte0ID) ? pf::UARTE0_GROUP : pf::UARTE1_GROUP;
            pinctrl::request_function(func_group);

            raw_writeptr(base_ + kRxdPtr, rx_buffer_);
            raw_write32(base_ + kRxdMaxCnt, sizeof(rx_buffer_));

            raw_writeptr(base_ + kTxdPtr, tx_buffer_);
            raw_write32(base_ + kTxdMaxCnt, sizeof(tx_buffer_));

            set_baudrate(kDefaultRate);

            // No Hw Flow, no parity, one stop bit
            uint32_t config_value = 0;

            if ((pinctrl::get_pin(func_group + pf::UARTE_CTS) > 0)
                && (pinctrl::get_pin(func_group + pf::UARTE_RTS) > 0)) {
                config_value |= kConfigHwFlow;
            }

            raw_write32(base_ + kConfig, config_value);

            configured_ = true;
            return 0;
        }

        unsigned int set_baudrate(unsigned int rate) override {
            uint32_t best_conf = 0;
            unsigned int best_rate = 0;
            for (size_t i = 0; i < ARRAY_SIZE(supported_rates); ++i) {
                if (ABSDIFF(rate, best_rate) > ABSDIFF(rate, supported_rates[i].actual_rate)) {
                    best_rate = supported_rates[i].actual_rate;
                    best_conf = supported_rates[i].conf_value;
                }
            }

            if (best_conf) {
                raw_write32(base_ + kBaud, best_conf);
            }

            return best_rate;
        }

        size_t write_str(const char* str) override {
            size_t transferred = 0;
            size_t amount = 0;
            unsigned int i = 0;
            while (*str) {
                tx_buffer_[i] = *str;
                ++amount;
                ++i;
                ++str;
                if (!*str || amount == sizeof(tx_buffer_)) {
                    raw_write32(base_ + kTxdMaxCnt, amount);
                    trigger_task(Task::START_TX);
                    busy_wait_and_clear_event(Event::END_TX);
                    transferred += amount;

                    if (!*str) {
                        break;
                    } else {
                        amount = 0;
                        i = 0;
                    }
                }
            }

            return transferred;
        }

    private:
        enum Task {
            START_RX,
            STOP_RX,
            START_TX,
            STOP_TX,
            FLUSH_RX = (0x02c >> 2),
        };

        enum Event {
            CTS,
            NCTS,
            RXRDY,
            RESERVED_1,
            END_RX,
            RESERVED_2 = END_RX + 2,
            TXRDY,
            END_TX,
            ERROR,
        };

        static constexpr auto kEnableOffset = 0x500;
        static constexpr auto kEnableValue = 8;

        static constexpr auto kPselRts = 0x508;
        static constexpr auto kPselTxd = 0x50c;
        static constexpr auto kPselCts = 0x510;
        static constexpr auto kPselRxd = 0x514;
        static constexpr auto kPselMask = 0x3f;
        static constexpr auto kPselZero = kPselRts - 4;
        static constexpr uint32_t kPselDisconnect = (1 << 31);

        static constexpr auto kRxdPtr = 0x534;
        static constexpr auto kRxdMaxCnt = 0x538;
        static constexpr auto kRxdAmount = 0x53c;

        static constexpr auto kTxdPtr = 0x544;
        static constexpr auto kTxdMaxCnt = 0x548;
        static constexpr auto kTxdAmount = 0x54c;

        static constexpr auto kBaud = 0x524;
        static constexpr auto kConfig = 0x56c;

        static constexpr auto kTxBufferSize = 32;
        static constexpr auto kRxBufferSize = 16;

        static constexpr auto kDefaultRate = 115200;
        static constexpr auto kConfigHwFlow = (1 << 4);

        static constexpr auto kEvtEndTx = 0x120;

        bool configured_ = false;

        uint8_t tx_buffer_[kTxBufferSize];
        uint8_t rx_buffer_[kRxBufferSize];

};

UARTE uarte0{kUarte0ID};
UARTE uarte1{kUarte1ID};

}  // namespace

UART* UART::request_by_id(UART::ID id) {
    UART* ret = nullptr;
    switch (id) {
    case ID::UARTE0:
        ret = &uarte0;
        break;
    case ID::UARTE1:
        ret = &uarte1;
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
