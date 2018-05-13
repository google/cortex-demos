#include "pinctrl.hpp"

#include "fake_board.hpp"

namespace pinctrl {

struct entry pinconfig_entries[] = {
    {
        .function = UARTE0_RXD,
        .pin = 13,
    },
    {
        .function = UARTE0_CTS,
        .pin = 12,
    },
    {
        .function = UARTE0_RTS,
        .pin = 13,
    },
    {
        .function = UARTE0_TXD,
        .pin = 32,
    },
};

struct config board_config = {
    .n_entries = sizeof(pinconfig_entries) / sizeof(pinconfig_entries[0]),
    .entries = pinconfig_entries,
};

struct config* board_pin_config = &board_config;

};
