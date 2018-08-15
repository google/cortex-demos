/*******************************************************************************
    Copyright 2018 Google LLC

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        https://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*******************************************************************************/

#include "third_party/catch2/catch.hpp"

#include <cstring>

#include "mock_memio.hpp"

#include "clk.h"
#include "sam4s/clk.h"
#include "driver/uart.hpp"

constexpr auto pmc_base = 0x400E0400;
constexpr auto pmc_pcer0 = pmc_base + 0x10;
constexpr auto ckgr_mor = pmc_base + 0x20;
constexpr auto ckgr_mcfr = pmc_base + 0x24;
constexpr auto ckgr_pllar = pmc_base + 0x28;
constexpr auto ckgr_pllbr = pmc_base + 0x2c;
constexpr auto pmc_mckr = pmc_base + 0x30;
constexpr auto pmc_sr = pmc_base + 0x68;
constexpr auto pmc_pcer1 = pmc_base + 0x100;

constexpr uint32_t uart0_base = 0x400e0600;
constexpr uint32_t uart1_base = 0x400e0800;

constexpr uint32_t uart_base(int index) {
    return index == 0 ? uart0_base : uart1_base;
}

constexpr uint32_t uart_cr(int index) {
    return uart_base(index);
}

constexpr uint32_t uart_sr(int index) {
    return uart_base(index) + 0x14;
}

constexpr uint32_t uart_thr(int index) {
    return uart_base(index) + 0x1c;
}

constexpr uint32_t uart_mr(int index) {
    return uart_base(index) + 0x4;
}

constexpr uint32_t uart_brgr(int index) {
    return uart_base(index) + 0x20;
}

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

            // Check that the default parity is no parity
            CHECK((mem.get_value_at(uart_mr(0)) & (7 << 9)) == (1 << 11));
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

            // Check that receiver and transmitter were enabled
            CHECK(mem.get_value_at(uart_cr(1)) == ((1 << 4) | (1 << 6)));
            // Check that the default parity is no parity
            CHECK((mem.get_value_at(uart_mr(1)) & (7 << 9)) == (1 << 11));
        }

        SECTION("Test Write") {
            auto* uart = driver::UART::request_by_id(driver::UART::ID::UART1);
            REQUIRE(uart != nullptr);

            mock::WriteSink<uint8_t> tx_data_handler;
            mem.set_addr_io_handler(uart_thr(1), &tx_data_handler);

            // For simplycity TXRDY will always read as one
            mem.set_value_at(uart_sr(1), (1 << 1));

            const std::string hello("Hello!");
            auto written = uart->write_str(hello.c_str());
            CHECK(written == hello.size());

            const auto& written_data = tx_data_handler.get_data();
            REQUIRE(written_data.size() == hello.size());
            CHECK(std::memcmp(&written_data[0], hello.c_str(), hello.size()) == 0);
        }
    }
}

TEST_CASE("Test Baud Rate Setting") {
    auto& mem = mock::get_global_memory();
    mem.reset();
    // Set reset values of some clock registers
    mem.set_value_at(ckgr_mor, 8);
    mem.set_value_at(ckgr_pllar, 0x3f00);
    mem.set_value_at(ckgr_pllbr, 0x3f00);
    mem.set_value_at(pmc_mckr, 1);

    auto pck_rate = clk_get_rate(SAM4S_CLK_PCK);
    REQUIRE(pck_rate > 0);

    SECTION("UART0") {
        auto* uart = driver::UART::request_by_id(driver::UART::ID::UART0);
        REQUIRE(uart != nullptr);

        unsigned br = uart->set_baudrate(14400);
        REQUIRE(br > 0);
        auto cd = mem.get_value_at(uart_brgr(0));
        REQUIRE(cd > 0);
        CHECK(br == pck_rate / (16 * cd));
    }

    SECTION("UART1") {
        auto* uart = driver::UART::request_by_id(driver::UART::ID::UART1);
        REQUIRE(uart != nullptr);

        unsigned br = uart->set_baudrate(14400);
        REQUIRE(br > 0);
        auto cd = mem.get_value_at(uart_brgr(1));
        REQUIRE(cd > 0);
        CHECK(br == pck_rate / (16 * cd));
    }
}
