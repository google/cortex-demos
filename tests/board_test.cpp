#include "catch.hpp"

#include "board.hpp"

namespace board {

int Config::get_pin(int function) {
    return function + 1;
}

}  // namespace board


TEST_CASE("Test Board Config API implementation") {
    board::Config cfg;
    CHECK(cfg.get_pin(3) == 4);
}
