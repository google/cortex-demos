#include "gpio.h"
#include "nvic.h"
#include "driver/timer.hpp"

extern "C" void __cxa_pure_virtual() {
    while (1);
}

namespace {

int counter = 0;

void toggle_leds() {
    ++counter;

    auto gpio_n = 17 + (counter & 3);
    gpio_toggle(0, (1 << gpio_n));
}

}

int main(void) {
    nvic_init();

    auto* rtc = driver::Timer::get_by_id(driver::Timer::ID::RTC0);

    rtc->set_prescaler(0xfff - 1);
    rtc->set_irq_handler(toggle_leds);
    rtc->enable_tick_interrupt();
    rtc->start();

    while (1);
}
