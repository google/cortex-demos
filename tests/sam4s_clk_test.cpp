#include "catch.hpp"

#include "mock_memio.hpp"

#include "clk.h"
#include "sam4s/clk.h"

constexpr auto pmc_base = 0x400E0400;
constexpr auto pmc_sr = pmc_base + 0x68;
constexpr auto ckgr_mor = pmc_base + 0x20;

TEST_CASE("Test Fast Clock Configuration") {
    auto& mem = mock::get_global_memory();

    mem.reset();

    mock::SourceIOHandler statuses;
    statuses.add_value(0);
    statuses.add_value(0);
    statuses.add_value(1);
    mem.set_addr_io_handler(pmc_sr, &statuses);

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
    status = mem.get_value_at(ckgr_mor);
    CHECK((status & mosc_rcen) == 0);
    CHECK((status & mosc_sel) == mosc_sel);
    CHECK((status & (0xff << 16)) == ck_passwd);
}
