#include "driver/uarte.hpp"

#include "memio.h"
#include "pinctrl.hpp"

#include "nrf52/periph_utils.hpp"
#include "nrf52/pinctrl.hpp"


namespace driver {

namespace {

// Offset for various registers


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

            configured_ = true;
            return 0;
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

        static constexpr auto kTxBufferSize = 32;
        static constexpr auto kRxBufferSize = 16;

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
