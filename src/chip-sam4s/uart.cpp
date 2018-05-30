#include "driver/uart.hpp"

#include "memio.h"

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
            // TODO: Do this from clock controller driver
            raw_write32(pmc_pcer0, (1 << irq_n_));

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

            configured_ = true;
            return 0;
        }

    private:
        static constexpr auto kCrOffset = 0;
        static constexpr uint32_t kCrTxEn = (1 << 6);
        static constexpr uint32_t kCrRxEn = (1 << 4);


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
