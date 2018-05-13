#include "driver/uarte.hpp"

#include "memio.h"
#include "nrf52/periph_utils.hpp"


namespace driver {

namespace {

class UARTE : public UART {
    public:
        UARTE(unsigned int id) : UART(periph::id_to_base(id), id) {}

        int request() override {
            if (configured_) {
                return 0;
            }

            raw_write32(base_ + kEnableOffset, kEnableValue);

            return 0;
        }

    private:
        static constexpr auto kEnableOffset = 0x500;
        static constexpr auto kEnableValue = 8;

        bool configured_ = false;

};

UARTE uarte0{2};
UARTE uarte1{40};

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
