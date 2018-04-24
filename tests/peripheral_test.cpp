#include "catch.hpp"

#include "mock_memio.hpp"

#include "cutils.h"
#include "nvic.h"
#include "driver/peripheral.hpp"

namespace {

static evt_handler_func_t evt_handlers[3];

class TestPeriph : public driver::Peripheral {
    public:
        TestPeriph() : Peripheral(0x40001000, 1, evt_handlers, ARRAY_SIZE(evt_handlers)) {}
        void enable_interrupts(uint32_t) override {}
        void disable_interrupts(uint32_t) override {}

        bool is_clear = false;

    private:
        bool is_event_active(int evt) override {
            if (evt == 1) {
                return true;
            }

            return false;
        }

        void clear_event(int evt) override {
            if (evt == 1) {
                is_clear = true;
            }
        }

} test_periph;

static void dummy_handler() {
    test_periph.handle_events();
}

static int counter = 0;

static void event_handler(int evt) {
    (void)evt;
    ++counter;
}

}  // namespace

TEST_CASE("Test IRQ Setting") {
    auto& mem = mock::get_global_memory();
    mem.reset();

    nvic_init();

    test_periph.set_irq_handler(dummy_handler);
    CHECK(test_periph.add_event_handler(1, event_handler) >= 0);
    CHECK(counter == 0);
    nvic_dispatch(test_periph.get_irq_num());
    CHECK(counter == 1);

    CHECK(test_periph.is_clear == true);
}
