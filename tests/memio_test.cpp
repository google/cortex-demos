#include "catch.hpp"

#include <cstdint>

#include "memio.h"
#include "mock_memio.hpp"

TEST_CASE("Test Memory IO read/write") {
    constexpr uint32_t test_addr = 0x1000 * 0x1000;
    constexpr uint32_t test_value = 0xdeadbeef;

    auto& mem = mock::get_global_memory();
    mem.reset();

    raw_write32(test_addr, test_value);
    CHECK(raw_read32(test_addr) == test_value);
}
