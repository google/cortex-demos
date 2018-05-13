#include "catch.hpp"

#include "fake_board.hpp"
#include "mock_memio.hpp"

#include "pinctrl.hpp"

TEST_CASE("Test UARTE pins configuration") {
    // Numbers are based on fake_board.cpp
    auto pin = pinctrl::get_pin(pinctrl::UARTE0_RXD);
    CHECK(pin == 13);

    pin = pinctrl::get_pin(pinctrl::UARTE0_CTS);
    CHECK(pin == 12);
}
