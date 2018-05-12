#include "driver/uarte.hpp"

#include "nrf52/periph_utils.hpp"

namespace driver {

namespace {

class UARTE : public UART {
    public:
        UARTE(unsigned int id) : UART(periph::id_to_base(id), id) {}

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

    return ret;
}

}  // namespace driver
