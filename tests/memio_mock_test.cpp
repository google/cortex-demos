#include "mock_memio.hpp"

#include "catch.hpp"

namespace {

class BitFlipStub : public mock::IOHandlerStub {
    public:
        uint32_t write32(uint32_t addr, uint32_t old_value, uint32_t new_value) override {
            (void)addr;
            (void)old_value;
            return ~new_value;
        }
};

}  // namespace

TEST_CASE("Test Memory IO stubbing") {
    mock::Memory mem;

    BitFlipStub bit_flipper;

    SECTION("Setting handler for a single address") {
        constexpr uint32_t test_addr = 0x10000000;

        mem.set_addr_io_handler(test_addr, &bit_flipper);

        constexpr uint32_t test_value = 0xdeadbeef;
        mem.write32(test_addr, test_value);
        CHECK(mem.get_value_at(test_addr) == (~test_value));
    }

    SECTION("Setting handler for a range") {
        constexpr uint32_t range_start = 0x100;
        constexpr uint32_t range_end = 0x200;

        mem.set_addr_io_handler(range_start, range_end, &bit_flipper);

        constexpr uint32_t test_value = 0xdecafbad;
        constexpr auto write_addr = range_start + 4 * 10;
        static_assert(write_addr < range_end, "Invalid test value");
        mem.write32(write_addr, 0xdecafbad);
        CHECK(mem.get_value_at(write_addr) == (~test_value));
    }
}

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
