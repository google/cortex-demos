#include "catch.hpp"

#include "mock_memio.hpp"

#include "pinctrl.hpp"

TEST_CASE("Test UARTE pins configuration") {
    // Numbers are based on fake_board.cpp
    auto pin = pinctrl::get_pin(1);
    CHECK(pin == 12);

    pin = pinctrl::get_pin(2);
    CHECK(pin == 32);
}
