#pragma once

namespace pinctrl {

enum function {
    UARTE_RTS = 1,
    UARTE_TXD = 2,
    UARTE_CTS = 3,
    UARTE_RXD = 4,
    MIN_PIN_FUNCTION,
    UARTE0_GROUP,
    UARTE0_RTS,
    UARTE0_TXD,
    UARTE0_CTS,
    UARTE0_RXD,
    UARTE1_GROUP,
    UARTE1_RTS,
    UARTE1_TXD,
    UARTE1_CTS,
    UARTE1_RXD,

    MAX_PIN_FUNCTION,
};

}  // namespace pinctrl
