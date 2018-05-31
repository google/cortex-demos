#include "catch.hpp"

#include "mock_memio.hpp"

#include "clk.h"
#include "sam4s/clk.h"

constexpr auto pmc_base = 0x400E0400;
constexpr auto pmc_sr = pmc_base + 0x68;
constexpr auto ckgr_mor = pmc_base + 0x20;
constexpr auto ckgr_mcfr = pmc_base + 0x24;
constexpr auto ckgr_pllar = pmc_base + 0x28;
constexpr auto ckgr_pllbr = pmc_base + 0x2c;
constexpr auto pmc_mckr = pmc_base + 0x30;

constexpr auto supc_base = 0x400e1410;
constexpr auto supc_cr = supc_base;
constexpr auto supc_sr = supc_base + 0x14;

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
    constexpr auto MHz = 1000 * 1000;
    // Set internal Fast RC as MAINCK for tests
    mem.set_value_at(ckgr_mor, (1 << 4) | (1 << 3));
    const uint32_t mainck_rate = clk_get_rate(SAM4S_CLK_MAINCK);
    CHECK(mainck_rate == 8 * MHz);

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
        CHECK(mem.get_op_count(mock::Memory::Op::READ32, pmc_sr) == 3);
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
        CHECK(mem.get_op_count(mock::Memory::Op::READ32, pmc_sr) == 3);
    }
}

TEST_CASE("Test Clock Rate calculation") {
    auto& mem = mock::get_global_memory();

    mem.reset();

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

        constexpr uint32_t pmc_sr_locka = (1 << 1);
        constexpr uint32_t pmc_sr_lockb = (1 << 2);

        uint32_t mul = 13;
        mem.set_value_at(ckgr_pllar, (mul << 16));
        mem.set_value_at(ckgr_pllbr, (mul << 16));

        CHECK(clk_get_rate(SAM4S_CLK_PLLACK) == 0);
        CHECK(clk_get_rate(SAM4S_CLK_PLLBCK) == 0);

        uint32_t div = 1;
        mem.set_value_at(ckgr_pllar, (1 << 29) | (mul << 16) | (div + 1));
        mem.set_value_at(ckgr_pllbr, ((mul + 3) << 16) | div);

        CHECK(clk_get_rate(SAM4S_CLK_PLLACK) == ((mainck_rate * (mul + 1)) / (div + 1)));
        CHECK(clk_get_rate(SAM4S_CLK_PLLBCK) == ((mainck_rate * (mul + 4)) / div));
    }
}
