#include "pinctrl.hpp"

#include "nrf52/pinctrl.hpp"

namespace pinctrl {

struct entry pinconfig_entries[] = {
    {
        .function = function::UARTE0_RXD,
        .pin = 8,
    },
    {
        .function = function::UARTE0_CTS,
        .pin = 7,
    },
    {
        .function = function::UARTE0_RTS,
        .pin = 5,
    },
    {
        .function = function::UARTE0_TXD,
        .pin = 6,
    },
};

struct config board_config = {
    .n_entries = sizeof(pinconfig_entries) / sizeof(pinconfig_entries[0]),
    .entries = pinconfig_entries,
};

struct config* board_pin_config = &board_config;

};
