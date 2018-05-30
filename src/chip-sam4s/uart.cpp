#include "driver/uart.hpp"

#include "memio.h"

constexpr auto pmc_pcer0 = 0x400e0410;

namespace driver {

namespace internal {

class UART : public ::driver::UART {
    public:
        UART(uint32_t base, unsigned int id) : driver::Peripheral(base, id) {}

        int request() override {
            if (configured_) {
                return 0;
            }

            raw_write32(pmc_pcer0, (1 << irq_n_));

            configured_ = true;
            return 0;
        }

    private:
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
