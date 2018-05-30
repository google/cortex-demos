#include "catch.hpp"

#include "mock_memio.hpp"

#include "driver/uart.hpp"

constexpr uint32_t uart0_base = 0x400e0600;
constexpr uint32_t uart1_base = 0x400e0800;

constexpr uint32_t uart_base(int index) {
    return index == 0 ? uart0_base : uart1_base;
}

constexpr uint32_t uart_cr(int index) {
    return uart_base(index);
}

constexpr uint32_t pmc_base = 0x400e0400;
constexpr uint32_t pmc_pcer(int index) {
    return pmc_base + (index == 0 ? 0x10 : 0x100);
}

constexpr uint32_t pmc_pcsr(int index) {
    return pmc_pcer(index) + 8;
}

constexpr uint32_t pio_base(int index) {
    return 0x400e0e00 + (index * 0x200);
}

constexpr uint32_t pio_per(int index) {
    return pio_base(index);
}

constexpr uint32_t pio_psr(int index) {
    return pio_per(index) + 8;
}

constexpr uint32_t pio_abcdsr1(int index) {
    return pio_base(index) + 0x70;
}

constexpr uint32_t pio_abcdsr2(int index) {
    return pio_base(index) + 0x74;
}

namespace {

class RegEDS : public mock::RegSetClearStub {
    public:
        RegEDS(uint32_t enable_reg) : mock::RegSetClearStub(enable_reg + 8, enable_reg, enable_reg + 4) {}

        static void install(mock::Memory& mem, RegEDS* eds) {
            mem.set_addr_io_handler(eds->set_addr_, eds->set_addr_ + 8, eds);
        }
};

}  // namespace

TEST_CASE("Test UART API") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    SECTION("Test Request") {
        RegEDS pclk0{pmc_pcer(0)};
        RegEDS::install(mem, &pclk0);

        RegEDS pclk1{pmc_pcer(1)};
        RegEDS::install(mem, &pclk1);

        RegEDS _per_a{pio_per(0)};
        RegEDS::install(mem, &_per_a);

        RegEDS _per_b{pio_per(1)};
        RegEDS::install(mem, &_per_b);

        SECTION("UART0") {
            auto* uart = driver::UART::request_by_id(driver::UART::ID::UART0);
            REQUIRE(uart != nullptr);
            const auto status = mem.get_value_at(pmc_pcsr(0));
            const auto uart_en_flag = (1 << 8);
            CHECK((status & uart_en_flag) == uart_en_flag);

            // Test Pin configuration
            // URXD0 -> PA9, Peripheral A
            // UTXD0 -> PA10, Peripheral A

            constexpr auto urxd = (1 << 9);
            constexpr auto utxd = (1 << 10);
            const auto io_status = mem.get_value_at(pio_psr(0));

            CHECK((io_status & urxd) == urxd);
            CHECK((io_status & utxd) == utxd);

            CHECK((mem.get_value_at(pio_abcdsr1(0)) & urxd) == 0);
            CHECK((mem.get_value_at(pio_abcdsr2(0)) & urxd) == 0);
            CHECK((mem.get_value_at(pio_abcdsr1(0)) & utxd) == 0);
            CHECK((mem.get_value_at(pio_abcdsr2(0)) & utxd) == 0);

            // Check that receiver and transmitter were enabled
            CHECK(mem.get_value_at(uart_cr(0)) == ((1 << 4) | (1 << 6)));
        }

        SECTION("UART1") {
            auto* uart = driver::UART::request_by_id(driver::UART::ID::UART1);
            REQUIRE(uart != nullptr);

            const auto status = mem.get_value_at(pmc_pcsr(0));
            const auto uart_en_flag = (1 << 9);
            CHECK((status & uart_en_flag) == uart_en_flag);

            // Test Pin configuration
            // URXD1 -> PB2, Peripheral A
            // UTXD1 -> PB3, Peripheral A
            constexpr auto urxd = (1 << 2);
            constexpr auto utxd = (1 << 3);
            const auto io_status = mem.get_value_at(pio_psr(1));

            CHECK((io_status & urxd) == urxd);
            CHECK((io_status & utxd) == utxd);

            CHECK((mem.get_value_at(pio_abcdsr1(1)) & urxd) == 0);
            CHECK((mem.get_value_at(pio_abcdsr2(1)) & urxd) == 0);
            CHECK((mem.get_value_at(pio_abcdsr1(1)) & utxd) == 0);
            CHECK((mem.get_value_at(pio_abcdsr2(1)) & utxd) == 0);

            CHECK(mem.get_value_at(uart_cr(1)) == ((1 << 4) | (1 << 6)));
        }
    }
}
