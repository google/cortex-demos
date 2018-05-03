#include "FreeRTOS.h"
#include "task.h"

#include "driver/timer.hpp"
#include "gpio.h"
#include "memio.h"
#include "nvic.h"


namespace {

extern "C" void xPortSysTickHandler(void);

void handle_rtc_tick_event(int) {
    xPortSysTickHandler();
}

extern "C" {

const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

void xPortPendSVHandler(void);
void vPortSVCHandler(void);

void vPortSetupTimerInterrupt(void) {
    auto* rtc = driver::Timer::get_by_id(driver::Timer::ID::RTC0);
    auto rate = rtc->get_rate();
    rtc->set_prescaler(rate / configTICK_RATE_HZ);
    rtc->add_event_handler(0, handle_rtc_tick_event);
    rtc->enable_tick_interrupt();
    rtc->start();
}

}  // extern "C"

portTASK_FUNCTION(blinker_thread, params) {
    (void)params;
    gpio_set_option(0, (1 << 17) | (1 << 18) | (1 << 19) | (1 << 20), GPIO_OPT_OUTPUT);

    int counter = 0;

    while (1) {
        const auto gpio_n = 17 + (counter++ & 3);
        gpio_toggle(0, (1 << gpio_n));
        vTaskDelay(pdMS_TO_TICKS(400));
    }
}

}  // namespace


int main() {
    nvic_init();

    nvic_set_handler(IRQ_SVCALL, vPortSVCHandler);
    nvic_set_handler(IRQ_PENDSV, xPortPendSVHandler);

    xTaskCreate(blinker_thread, "BLINK", 10 * configMINIMAL_STACK_SIZE,
            /* params = */NULL, tskIDLE_PRIORITY + 1, NULL);

    vTaskStartScheduler();

    while (1) ;
}
