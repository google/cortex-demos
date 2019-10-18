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

#include "clk.h"
#include "memio.h"

#include "sam4s/clk.h"

constexpr auto pmc_pcer0 = 0x400e0410;
constexpr auto pio_pera = 0x400e0e00;
constexpr auto pio_perb = 0x400e1000;
constexpr auto pio_abcdsr0a = pio_pera + 0x70;
constexpr auto pio_abcdsr0b = pio_perb + 0x70;

namespace driver {

namespace internal {

class UART : public ::driver::UART {
    public:
        UART(uint32_t base, unsigned int id) : driver::Peripheral(base, id) {}

        int request() override {
            if (configured_) {
                return 0;
            }

            // Enable peripheral clock
            // TODO: Move this to common peripheral class
            clk_request(SAM4S_CLK_PIDCK(irq_n_));

            // Configure IO lines.
            // TODO: Do this from pinctrl driver
            const auto per = irq_n_ == 8 ? pio_pera : pio_perb;
            const auto urxd = irq_n_ == 8 ? (1 << 9) : (1 << 2);
            const auto utxd = urxd << 1;

            const auto abcdsr0 = irq_n_ == 8 ? pio_abcdsr0a : pio_abcdsr0b;

            raw_write32(per, (urxd | utxd));
            raw_clrbits_le32(abcdsr0, (urxd | utxd));
            raw_clrbits_le32(abcdsr0 + 4, (urxd | utxd));

            //enable transmitter and receiver
            raw_write32(base_ + kCrOffset, (kCrTxEn | kCrRxEn));
            set_parity(PARITY::NONE);

            configured_ = true;
            return 0;
        }

        unsigned int set_baudrate(unsigned int rate) override {
            if (rate == 0) {
                return 0;
            }
            // TODO: This should be a request for parent clock rate,
            // this driver does not need to hardcode its parent.
            unsigned int input_rate = clk_get_rate(SAM4S_CLK_PCK);
            if (input_rate == 0) {
                return 0;
            }
            uint32_t cd = input_rate / (16 * rate);
            if (cd == 0) {
                return 0;
            }

            raw_write32(base_ + kBrgrOffset, cd);
            unsigned int br = input_rate / (16 * cd);
            return br;
        }

        int set_parity(PARITY parity) override {
            uint32_t parity_value = 0;
            uint32_t mr = raw_read32(base_ + kMrOffset);
            switch (parity) {
            case PARITY::NONE:
                parity_value = 4;
                break;
            case PARITY::EVEN:
                parity_value = 0;
                break;
            case PARITY::ODD:
                parity_value = 1;
                break;
            }

            mr &= ~(kMrParMask);
            mr |= (parity_value << kMrParShift);

            raw_write32(base_ + kMrOffset, mr);

            return 0;
        }

        size_t write_str(const char* str) override {
            size_t ret = 0;
            while (*str) {
                wait_mask_le32(base_ + kStatusOffset, kStatusTxRdy);
                raw_write32(base_ + kThrOffset, *str);
                ++str;
                ++ret;
            }

            return ret;
        }

    private:
        static constexpr auto kCrOffset = 0;
        static constexpr uint32_t kCrTxEn = (1 << 6);
        static constexpr uint32_t kCrRxEn = (1 << 4);

        static constexpr auto kMrOffset = 4;
        static constexpr uint32_t kMrParMask = 7;
        static constexpr uint32_t kMrParShift = 9;

        static constexpr auto kStatusOffset = 0x14;
        static constexpr auto kStatusTxRdy = (1 << 1);

        static constexpr auto kThrOffset = 0x1c;

        static constexpr auto kBrgrOffset = 0x20;


        bool configured_ = false;
};

UART uart0{0x400E0600, 8};
UART uart1{0x400E0800, 9};

}  // internal

UART* UART::request_by_id(UART::ID id) {
    UART* ret = nullptr;
    switch (id) {
    case ID::UART0:
        ret = &internal::uart0;
        break;
    case ID::UART1:
        ret = &internal::uart1;
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
