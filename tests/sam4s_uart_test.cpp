#include "catch.hpp"

#include "mock_memio.hpp"

#include "driver/uart.hpp"

constexpr auto pmc_base = 0x400e0400;
constexpr auto pmc_pcer(int index) {
    return pmc_base + (index == 0 ? 0x10 : 0x100);
}

constexpr auto pmc_pcsr(int index) {
    return pmc_pcer(index) + 8;
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
        RegEDS pclk1{pmc_pcer(1)};

        RegEDS::install(mem, &pclk0);
        RegEDS::install(mem, &pclk1);

        int index = 0;
        for (auto dev_id : {driver::UART::ID::UART0, driver::UART::ID::UART1}) {
            auto* uart = driver::UART::request_by_id(dev_id);
            REQUIRE(uart != nullptr);

            const auto status = mem.get_value_at(pmc_pcsr(0));
            const auto uart_en_flag = (1 << (index + 8));
            CHECK((status & uart_en_flag) == uart_en_flag);

            ++index;
        }
    }
}
