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

#include "mock_memio.hpp"

#include "clk.h"
#include "sam4s/clk.h"

constexpr auto pmc_base = 0x400E0400;
constexpr auto pmc_pcer0 = pmc_base + 0x10;
constexpr auto ckgr_mor = pmc_base + 0x20;
constexpr auto ckgr_mcfr = pmc_base + 0x24;
constexpr auto ckgr_pllar = pmc_base + 0x28;
constexpr auto ckgr_pllbr = pmc_base + 0x2c;
constexpr auto pmc_mckr = pmc_base + 0x30;
constexpr auto pmc_sr = pmc_base + 0x68;
constexpr auto pmc_pcer1 = pmc_base + 0x100;

constexpr auto supc_base = 0x400e1410;
constexpr auto supc_cr = supc_base;
constexpr auto supc_sr = supc_base + 0x14;

namespace {

// TODO: move this to common sam4s testing library
class RegEDS : public mock::RegSetClearStub {
    public:
        RegEDS(uint32_t enable_reg) : mock::RegSetClearStub(enable_reg + 8, enable_reg, enable_reg + 4) {}

        static void install(mock::Memory& mem, RegEDS* eds) {
            mem.set_addr_io_handler(eds->set_addr_, eds->set_addr_ + 8, eds);
        }
};

}  // namespace

TEST_CASE("Test Fast Clock Configuration") {
    auto& mem = mock::get_global_memory();

    mem.reset();

    mock::SourceIOHandler statuses;
    statuses.add_value(0);
    statuses.add_value(0);
    statuses.add_value(1);
    mem.set_addr_io_handler(pmc_sr, &statuses);

    mock::SourceIOHandler mcfr;
    mcfr.add_value(0);
    mcfr.add_value(0);
    mcfr.add_value((1 << 16));
    // For built-int slow clock this corresponds to 16MHz
    mcfr.add_value((1 << 16) | 8000);
    mem.set_addr_io_handler(ckgr_mcfr, &mcfr);

    constexpr auto mosc_rcen = (1 << 3);
    mem.set_value_at(ckgr_mor, mosc_rcen);

    REQUIRE(clk_request(SAM4S_CLK_HF_CRYSTAL) >= 0);

    constexpr auto ck_passwd = (0x37 << 16);
    constexpr auto mosc_xten = (1 << 0);
    uint32_t status = mem.get_value_at(ckgr_mor);
    CHECK((status & mosc_rcen) == mosc_rcen);
    CHECK((status & mosc_xten) == mosc_xten);
    CHECK((status & (0xff << 16)) == ck_passwd);
    CHECK((status & (0xff << 8)) > 0);

    statuses.add_value(0);
    statuses.add_value(0);
    statuses.add_value((1 << 16));
    mem.set_value_at(ckgr_mor, mosc_rcen | mosc_xten);
    constexpr auto mosc_sel = (1 << 24);
    REQUIRE(clk_request(SAM4S_CLK_MAINCK) >= 0);
    CHECK(clk_get_rate(SAM4S_CLK_HF_CRYSTAL) == 16 * 1000 * 1000);
    status = mem.get_value_at(ckgr_mor);
    CHECK((status & mosc_rcen) == 0);
    CHECK((status & mosc_sel) == mosc_sel);
    CHECK((status & (0xff << 16)) == ck_passwd);
}

TEST_CASE("Test Slow Clock Request") {
    auto& mem = mock::get_global_memory();

    mem.reset();

    mem.set_value_at(supc_sr, (1 << 7));
    mock::SourceIOHandler statuses;
    statuses.add_value(0);
    statuses.add_value(0);
    mem.set_addr_io_handler(supc_sr, &statuses);

    REQUIRE(clk_request(SAM4S_CLK_XTAL_EXT) >= 0);
    CHECK(mem.get_value_at(supc_cr) == (0xa5000008));
}

TEST_CASE("Test PLL Configuration") {
    constexpr uint32_t pmc_sr_locka = (1 << 1);
    constexpr uint32_t pmc_sr_lockb = (1 << 2);
    auto& mem = mock::get_global_memory();

    mem.reset();
    // Set reset values of some clock registers
    mem.set_value_at(ckgr_mor, 8);
    mem.set_value_at(ckgr_pllar, 0x3f00);
    mem.set_value_at(ckgr_pllbr, 0x3f00);
    mem.set_value_at(pmc_mckr, 1);

    // Fake start crystal at 16MHz
    // For built-int slow clock this corresponds to 16MHz
    mem.set_value_at(ckgr_mcfr, (1 << 16) | 8000);
    mem.set_value_at(pmc_sr, 1 | (1 << 16));

    constexpr auto mosc_rcen = (1 << 3);
    mem.set_value_at(ckgr_mor, mosc_rcen);

    REQUIRE(clk_request(SAM4S_CLK_HF_CRYSTAL) >= 0);
    REQUIRE(clk_request(SAM4S_CLK_MAINCK) >= 0);

    constexpr auto MHz = 1000 * 1000;
    // based on one of real world problems
    unsigned crystal_freq = 11999232;
    sam4s_set_crystal_frequency(crystal_freq);
    // Set internal Fast RC as MAINCK for tests
    const uint32_t mainck_rate = clk_get_rate(SAM4S_CLK_MAINCK);
    CHECK(mainck_rate == crystal_freq);

    auto start_count = mem.get_op_count(mock::Memory::Op::READ32, pmc_sr);
    SECTION("PLL A") {
        mem.set_value_at(pmc_sr, pmc_sr_locka);
        mock::SourceIOHandler statuses;
        statuses.add_value(0);
        statuses.add_value(0);
        mem.set_addr_io_handler(pmc_sr, &statuses);

        unsigned rate = 120 * MHz;
        unsigned res_rate = clk_request_rate(SAM4S_CLK_PLLACK, rate);
        CHECK(res_rate > 0);

        auto pllr = mem.get_value_at(ckgr_pllar);
        auto mul = (pllr >> 16) & 0x7ff;
        auto div = (pllr & 0xff);
        REQUIRE(div > 0);
        unsigned rate_set = (mainck_rate * (mul + 1)) / div;
        CHECK(rate_set == res_rate);

        // Check that ONE is set
        CHECK((pllr & (1 << 29)) == (1 << 29));

        // Check that startup ticks is set
        CHECK((pllr & 0x3f00) > 0);

        // Check that the lock was waited on
        CHECK(mem.get_op_count(mock::Memory::Op::READ32, pmc_sr) == start_count + 3);
    }

    SECTION("PLL B") {
        mem.set_value_at(pmc_sr, pmc_sr_lockb);
        mock::SourceIOHandler statuses;
        statuses.add_value(0);
        statuses.add_value(0);
        mem.set_addr_io_handler(pmc_sr, &statuses);

        unsigned rate = 120 * MHz;
        unsigned res_rate = clk_request_rate(SAM4S_CLK_PLLBCK, rate);
        CHECK(res_rate > 0);

        auto pllr = mem.get_value_at(ckgr_pllbr);
        auto mul = (pllr >> 16) & 0x7ff;
        auto div = (pllr & 0xff);
        REQUIRE(div > 0);
        unsigned rate_set = (mainck_rate * (mul + 1)) / div;
        CHECK(rate_set == res_rate);

        // Check that ONE is set
        CHECK((pllr & (1 << 29)) == 0);

        // Check that startup ticks is set
        CHECK((pllr & 0x3f00) > 0);
        // Check that the lock was waited on
        CHECK(mem.get_op_count(mock::Memory::Op::READ32, pmc_sr) == start_count + 3);
    }
}

TEST_CASE("Test Master Clock Switch to PLL") {
    auto& mem = mock::get_global_memory();

    mem.reset();
    // The reset value of these registers
    mem.set_value_at(pmc_mckr, 1);
    mem.set_value_at(ckgr_mor, (1 << 3));

    SECTION("Switch to PLLA") {
        // PLL is stopped, should not be able to switch first.
        CHECK(clk_request_option(SAM4S_CLK_MCK, SAM4S_CLK_PLLACK) < 0);

        mem.set_value_at(ckgr_pllar, (1 << 29) | (10 << 16) | 1);
        // Self-Check: this was enought to enable the PLL
        auto pll_rate = clk_get_rate(SAM4S_CLK_PLLACK);
        REQUIRE(pll_rate > 0);

        mock::SourceIOHandler statuses;
        mem.set_value_at(pmc_sr, (1 << 3));
        statuses.add_value(0);
        statuses.add_value(0);
        statuses.add_value(0);
        mem.set_addr_io_handler(pmc_sr, &statuses);

        CHECK(clk_request_option(SAM4S_CLK_MCK, SAM4S_CLK_PLLACK) >= 0);
        CHECK(clk_get_rate(SAM4S_CLK_MCK) == pll_rate);
        CHECK(mem.get_op_count(mock::Memory::Op::READ32, pmc_sr) == 4);
    }

    SECTION("Switch to PLLB") {
        // PLL is stopped, should not be able to switch first.
        CHECK(clk_request_option(SAM4S_CLK_MCK, SAM4S_CLK_PLLBCK) < 0);

        mem.set_value_at(ckgr_pllbr, (1 << 29) | (10 << 16) | 1);
        // Self-Check: this was enought to enable the PLL
        auto pll_rate = clk_get_rate(SAM4S_CLK_PLLBCK);
        REQUIRE(pll_rate > 0);

        mock::SourceIOHandler statuses;
        mem.set_value_at(pmc_sr, (1 << 3));
        statuses.add_value(0);
        statuses.add_value(0);
        statuses.add_value(0);
        mem.set_addr_io_handler(pmc_sr, &statuses);

        CHECK(clk_request_option(SAM4S_CLK_MCK, SAM4S_CLK_PLLBCK) >= 0);
        CHECK(clk_get_rate(SAM4S_CLK_MCK) == pll_rate);
        CHECK(mem.get_op_count(mock::Memory::Op::READ32, pmc_sr) == 4);
    }
}

TEST_CASE("Test Clock Rate calculation") {
    auto& mem = mock::get_global_memory();

    mem.reset();
    mem.set_value_at(ckgr_mor, 8);
    mem.set_value_at(ckgr_pllar, 0x3f00);
    mem.set_value_at(ckgr_pllbr, 0x3f00);
    mem.set_value_at(pmc_mckr, 1);

    SECTION("Slow Clocks") {
        CHECK(clk_get_rate(SAM4S_CLK_XTAL_RC) == 32000);
        CHECK(clk_get_rate(SAM4S_CLK_XTAL_EXT) == 32768);

        CHECK(clk_get_rate(SAM4S_CLK_SLCK) == clk_get_rate(SAM4S_CLK_XTAL_RC));
        mem.set_value_at(supc_sr, (1 << 7));
        CHECK(clk_get_rate(SAM4S_CLK_SLCK) == clk_get_rate(SAM4S_CLK_XTAL_EXT));
    }

    SECTION("Main Clock and HF Oscillators") {
        // By default Fast RC is 4MHz
        mem.set_value_at(ckgr_mor, 0);
        CHECK(clk_get_rate(SAM4S_CLK_FAST_RC) == 0);

        constexpr auto MHz = 1000 * 1000;
        mem.set_value_at(ckgr_mor, (1 << 3));
        CHECK(clk_get_rate(SAM4S_CLK_FAST_RC) == 4 * MHz);
        mem.set_value_at(ckgr_mor, (1 << 4) | (1 << 3));
        CHECK(clk_get_rate(SAM4S_CLK_FAST_RC) == 8 * MHz);
        CHECK(clk_get_rate(SAM4S_CLK_MAINCK) == clk_get_rate(SAM4S_CLK_FAST_RC));
        mem.set_value_at(ckgr_mor, (1 << 5) | (1 << 3));
        CHECK(clk_get_rate(SAM4S_CLK_FAST_RC) == 12 * MHz);
        CHECK(clk_get_rate(SAM4S_CLK_MAINCK) == clk_get_rate(SAM4S_CLK_FAST_RC));

        mock::SourceIOHandler statuses;
        statuses.add_value(0);
        statuses.add_value(0);
        statuses.add_value(1);
        mem.set_addr_io_handler(pmc_sr, &statuses);

        mock::SourceIOHandler mcfr;
        mcfr.add_value(0);
        mcfr.add_value(0);
        mcfr.add_value((1 << 16));
        // For built-int slow clock this corresponds to 16MHz
        mcfr.add_value((1 << 16) | 8000);
        mem.set_addr_io_handler(ckgr_mcfr, &mcfr);

        constexpr auto mosc_rcen = (1 << 3);
        constexpr auto mosc_xten = (1 << 0);
        mem.set_value_at(ckgr_mor, mosc_rcen);

        REQUIRE(clk_request(SAM4S_CLK_HF_CRYSTAL) >= 0);
        statuses.add_value(0);
        statuses.add_value(0);
        statuses.add_value((1 << 16));
        mem.set_value_at(ckgr_mor, mosc_rcen | mosc_xten);
        REQUIRE(clk_request(SAM4S_CLK_MAINCK) >= 0);
        CHECK(clk_get_rate(SAM4S_CLK_HF_CRYSTAL) == 16 * 1000 * 1000);
        CHECK(clk_get_rate(SAM4S_CLK_MAINCK) == clk_get_rate(SAM4S_CLK_HF_CRYSTAL));

        // Master Clock is Main Clock by default
        CHECK(clk_get_rate(SAM4S_CLK_MCK) == clk_get_rate(SAM4S_CLK_MAINCK));

        unsigned int base_mcr_rate = clk_get_rate(SAM4S_CLK_MCK);
        // Test Divisors
        for (unsigned i = 0; i < 7; ++i) {
            mem.set_value_at(pmc_mckr, (i << 4) | 1);
            CHECK(clk_get_rate(SAM4S_CLK_MCK) == base_mcr_rate / (1 << i));
        }

        mem.set_value_at(pmc_mckr, (7 << 4) | 1);
        CHECK(clk_get_rate(SAM4S_CLK_MCK) == base_mcr_rate / 3);
        CHECK(clk_get_rate(SAM4S_CLK_HCLK) == base_mcr_rate / 3);
    }

    SECTION("Test Peripheral Clocks") {
        RegEDS pmc_pcr0{pmc_pcer0};
        RegEDS pmc_pcr1{pmc_pcer1};

        RegEDS::install(mem, &pmc_pcr0);
        RegEDS::install(mem, &pmc_pcr1);

        // Self test: main clock is configured
        auto mck_rate = clk_get_rate(SAM4S_CLK_MCK);
        REQUIRE(mck_rate > 0);

        CHECK(clk_get_rate(SAM4S_CLK_PCK) == mck_rate);
        CHECK(clk_get_rate(SAM4S_CLK_PIDCK(15)) == 0);

        CHECK(clk_request(SAM4S_CLK_PIDCK(15)) >= 0);
        CHECK(clk_request(SAM4S_CLK_PIDCK(31)) >= 0);
        CHECK((mem.get_value_at(pmc_pcer0 + 8) & (1 << 15)) == (1 << 15));
        CHECK(clk_get_rate(SAM4S_CLK_PIDCK(15)) == mck_rate);
        CHECK(clk_get_rate(SAM4S_CLK_PIDCK(31)) == mck_rate);

        CHECK(clk_request(SAM4S_CLK_PIDCK(33)) >= 0);
        CHECK(clk_request(SAM4S_CLK_PIDCK(32)) >= 0);
        CHECK((mem.get_value_at(pmc_pcer1 + 8) & (1 << 1)) == (1 << 1));
        CHECK((mem.get_value_at(pmc_pcer1 + 8) & (1 << 0)) == (1 << 0));
        CHECK(clk_get_rate(SAM4S_CLK_PIDCK(33)) == mck_rate);
        CHECK(clk_get_rate(SAM4S_CLK_PIDCK(32)) == mck_rate);
    }

    SECTION("Test PLLs") {
        constexpr auto MHz = 1000 * 1000;
        // Set internal Fast RC as MAINCK for tests
        mem.set_value_at(ckgr_mor, (1 << 4) | (1 << 3));
        const uint32_t mainck_rate = clk_get_rate(SAM4S_CLK_MAINCK);
        CHECK(mainck_rate == 8 * MHz);

        // Initially both PLLs are stopped
        CHECK(clk_get_rate(SAM4S_CLK_PLLACK) == 0);
        CHECK(clk_get_rate(SAM4S_CLK_PLLBCK) == 0);

        uint32_t mul = 13;
        mem.set_value_at(ckgr_pllar, (mul << 16));
        mem.set_value_at(ckgr_pllbr, (mul << 16));

        // No divisor -> still stopped
        CHECK(clk_get_rate(SAM4S_CLK_PLLACK) == 0);
        CHECK(clk_get_rate(SAM4S_CLK_PLLBCK) == 0);

        uint32_t div = 1;
        mem.set_value_at(ckgr_pllar, (1 << 29) | (mul << 16) | (div + 1));
        mem.set_value_at(ckgr_pllbr, ((mul + 3) << 16) | div);

        auto plla_input_rate = ((mainck_rate * (mul + 1)) / (div + 1));
        auto pllb_input_rate = ((mainck_rate * (mul + 4)) / div);
        CHECK(clk_get_rate(SAM4S_CLK_PLLACK) == plla_input_rate);
        CHECK(clk_get_rate(SAM4S_CLK_PLLBCK) == pllb_input_rate);

        // Enable divider
        mem.set_value_at(pmc_mckr, (1 << 12));
        CHECK(clk_get_rate(SAM4S_CLK_PLLACK) == plla_input_rate / 2);

        mem.set_value_at(pmc_mckr, (1 << 13));
        CHECK(clk_get_rate(SAM4S_CLK_PLLBCK) == pllb_input_rate / 2);
    }
}
