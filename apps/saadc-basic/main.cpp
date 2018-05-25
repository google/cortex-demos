#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

#include "core/thread.hpp"
#include "driver/timer.hpp"
#include "driver/uarte.hpp"
#include "driver/adc.hpp"
#include "gpio.h"
#include "memio.h"
#include "nvic.h"

// TODO: Move to non-app specific location
void operator delete(void*) {}
void operator delete(void*, unsigned) {}

namespace {

uint8_t alloc_buffer[100];

extern "C" void* _sbrk(int incr) {
    (void)incr;
    return alloc_buffer;
}

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

// TODO: Move to non-app specific location
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

}  // extern "C"

class BlinkerThread : public os::ThreadStatic<2 * configMINIMAL_STACK_SIZE> {
    public:
        BlinkerThread() : os::ThreadStatic<2 * configMINIMAL_STACK_SIZE>("BLINK", tskIDLE_PRIORITY + 1) {}
        void run() override {
            gpio_set_option(0, (1 << 17) | (1 << 18) | (1 << 19) | (1 << 20), GPIO_OPT_OUTPUT);

            int counter = 0;

            while (1) {
                ++counter;
                const auto gpio_n = 17 + (counter++ & 3);
                gpio_toggle(0, (1 << gpio_n));
                vTaskDelay(pdMS_TO_TICKS(400));
            }
        }
} blinker_thread;

class ADCThread : public os::ThreadStatic<16 * configMINIMAL_STACK_SIZE> {
    public:
        ADCThread() : os::ThreadStatic<16 * configMINIMAL_STACK_SIZE>("ADC", tskIDLE_PRIORITY + 2) {}
        void run() override {

            auto* uart = driver::UART::request_by_id(driver::UART::ID::UARTE0);

            auto* adc = driver::ADC::request_by_id(driver::ADC::ID::ADC0);
            configASSERT(adc);



            while (1) {
                vTaskDelay(pdMS_TO_TICKS(2000));
                auto res = adc->start(3);
                if (res > 0) {
                    snprintf(debug_str, sizeof(debug_str), "NRES: %d\r\n", res);
                    uart->write_str(debug_str);
                    auto ch0 = adc->get_result(0, 0);
                    auto ch1 = adc->get_result(1, 0);
                    snprintf(debug_str, sizeof(debug_str), "CH0-0: %lu, CH1-0: %lu\r\n", ch0, ch1);
                    uart->write_str(debug_str);

                    ch0 = adc->get_result(0, 1);
                    ch1 = adc->get_result(1, 1);
                    snprintf(debug_str, sizeof(debug_str), "CH0-1: %lu, CH1-1: %lu\r\n", ch0, ch1);
                    uart->write_str(debug_str);
                } else {
                    uart->write_str("FAIL\r\n");
                }
            }
        }

    private:
        char debug_str[80];
} adc_thread;

}  // namespace


int main() {
    nvic_init();

    nvic_set_handler(IRQ_SVCALL, vPortSVCHandler);
    nvic_set_handler(IRQ_PENDSV, xPortPendSVHandler);

    blinker_thread.init();
    adc_thread.init();

    auto* uart = driver::UART::request_by_id(driver::UART::ID::UARTE0);

    uart->write_str("Start\r\n");

    vTaskStartScheduler();

    while (1) ;
}
