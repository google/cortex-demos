#include "mock_memio.hpp"

#include "catch.hpp"

TEST_CASE("Test Basic Memory RW") {
    mock::Memory mem;

    constexpr auto value32 = 0xdeadbeef;
    constexpr auto test_addr = 0x40000000;

    CHECK(mem.read32(test_addr) == 0);
    CHECK(mem.read16(test_addr) == 0);
    CHECK(mem.read8(test_addr) == 0);
    mem.write32(test_addr, value32);
    CHECK(mem.read32(test_addr) == value32);

    // NOTE: This assumes Little Endian platform
    CHECK(mem.read16(test_addr) == (value32 & 0xffff));
    CHECK(mem.read16(test_addr + 2) == (value32 >> 16));

    CHECK(mem.read8(test_addr) == (value32 & 0xff));
    CHECK(mem.read8(test_addr + 1) == ((value32 >> 8) & 0xff));
    CHECK(mem.read8(test_addr + 2) == ((value32 >> 16) & 0xff));
    CHECK(mem.read8(test_addr + 3) == ((value32 >> 24) & 0xff));

    mem.write16(test_addr, 0xcafe);

    constexpr auto test_value2 = (value32 & (~0xffff)) | 0xcafe;
    CHECK(mem.read32(test_addr) == test_value2);

    mem.reset();
    CHECK(mem.read32(test_addr) == 0);
}

TEST_CASE("Test Memory Journal") {
    mock::Memory mem;

    constexpr auto test_addr = 0x2000 * 0x10000;
    CHECK(mem.read16(0x20000000) == 0);

    const auto& journal = mem.get_journal();
    const auto entry_it = std::find_if(journal.begin(), journal.end(),
            [](const mock::Memory::JournalT::value_type& entry) {
                return std::get<0>(entry) == mock::Memory::Op::READ16;
            });

    CHECK(entry_it != journal.end());
}
