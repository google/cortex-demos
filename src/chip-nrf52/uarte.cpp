#include "driver/uarte.hpp"

#include "cutils.h"
#include "memio.h"
#include "pinctrl.hpp"

#include "nrf52/periph_utils.hpp"
#include "nrf52/pinctrl.hpp"


namespace driver {

namespace {

// Offset for various registers

struct rate_config {
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

class UARTE : public UART {
    public:
        UARTE(unsigned int id) : UART(periph::id_to_base(id), id) {}

        int request() override {
            if (configured_) {
                return 0;
            }

            raw_write32(base_ + kEnableOffset, kEnableValue);

            using pf = pinctrl::function;
            auto func_group = base_ == periph::id_to_base(kUarte0ID) ? pf::UARTE0_GROUP : pf::UARTE1_GROUP;

            for (int i = pf::UARTE_RTS; i <= pf::UARTE_RXD; ++i) {
                auto pin_sel = pinctrl::request_function(func_group + i);
                if (pin_sel > 0) {
                    raw_write32(base_ + kPselZero + 4 * i, (pin_sel & kPselMask));
                }
            }

            raw_writeptr(base_ + kRxdPtr, rx_buffer_);
            raw_write32(base_ + kRxdMaxCnt, sizeof(rx_buffer_));

            raw_writeptr(base_ + kTxdPtr, tx_buffer_);
            raw_write32(base_ + kTxdMaxCnt, sizeof(tx_buffer_));

            set_baudrate(kDefaultRate);

            // No Hw Flow, no parity, one stop bit
            uint32_t config_value = 1;

            if ((pinctrl::get_pin(func_group + pf::UARTE_CTS) > 0)
                    || (pinctrl::get_pin(func_group + pf::UARTE_CTS) > 0)) {
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
                    raw_write32(base_ + kTaskStartTx, 1);
                    while (!raw_read32(base_ + kEvtEndTx));
                    raw_write32(base_ + kEvtEndTx, 0);
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
        static constexpr auto kEnableOffset = 0x500;
        static constexpr auto kEnableValue = 8;

        static constexpr auto kPselRts = 0x508;
        static constexpr auto kPselTxd = 0x50c;
        static constexpr auto kPselCts = 0x510;
        static constexpr auto kPselRxd = 0x514;
        static constexpr auto kPselMask = 0x3f;
        static constexpr auto kPselZero = kPselRts - 4;

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

        static constexpr auto kTaskStartTx = 0x8;

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
