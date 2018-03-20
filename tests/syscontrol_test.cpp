#include "catch.hpp"

#include <cstdint>

#include "mock_memio.hpp"

#include "syscontrol.h"

constexpr uint32_t vtor_addr = 0xe000ed08;

TEST_CASE("Test Vector Table relocation") {
    constexpr uint32_t new_location = 0x2000 << 16;
    constexpr uint32_t old_location = 0x1000 << 16;

    auto& mem = mock::get_global_memory();

    mem.set_value_at(vtor_addr, old_location);
    mem.set_value_at(old_location, 0xaa55aa55);
    mem.set_value_at(old_location + 4, 0x55aa55aa);
    mem.set_value_at(old_location + 8, 0x00aa00aa);

    syscontrol_relocate_vt(new_location, 2);
    auto vt_location = mem.get_value_at(vtor_addr);
    CHECK(vt_location == new_location);
    CHECK(mem.get_value_at(new_location) == 0xaa55aa55);
    CHECK(mem.get_value_at(new_location + 4) == 0x55aa55aa);
    CHECK(mem.get_value_at(new_location + 8) == 0x00aa00aa);
}
