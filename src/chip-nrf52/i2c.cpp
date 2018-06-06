#include "driver/i2c.hpp"

#include "cutils.h"
#include "memio.h"
#include "pinctrl.hpp"

#include "nrf52/peripheral.hpp"
#include "nrf52/periph_utils.hpp"
#include "nrf52/pinctrl.hpp"


namespace driver {

namespace {

constexpr auto kTwim0ID = 3;
constexpr auto kTwim1ID = 4;

class TWIM : public I2C, public nrf52::Peripheral {
    public:
        TWIM(unsigned int id) : driver::Peripheral(periph::id_to_base(id), id) {}

        int request() override {
            return 0;
        }
};

TWIM twim0{kTwim0ID};
TWIM twim1{kTwim1ID};

}  // namespace

I2C* I2C::request_by_id(int id) {
    I2C* res = nullptr;
    switch(id) {
        case nrf52::TWIM0:
            res = &twim0;
            break;
        case nrf52::TWIM1:
            res = &twim1;
            break;
    }

    if (res) {
        res->request();
    }

    return res;
}

}  // namespace driver
