#include "FreeRTOS.h"
#include "task.h"

#include "core/thread.hpp"
#include "gpio.h"
#include "memio.h"
#include "nvic.h"

#include "sam4s/gpio.h"

namespace {

extern "C" void xPortSysTickHandler(void);

extern "C" {

const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

void xPortPendSVHandler(void);
void vPortSVCHandler(void);

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
            auto led0_port = GPIO_PORTC;
            auto led0_mask = (1 << 23);

            gpio_set_option(led0_port, led0_mask, GPIO_OPT_OUTPUT);

            while (1) {
                vTaskDelay(pdMS_TO_TICKS(500));
                gpio_toggle(led0_port, led0_mask);
                vTaskDelay(pdMS_TO_TICKS(1000));
                gpio_toggle(led0_port, led0_mask);
                vTaskDelay(pdMS_TO_TICKS(750));
                gpio_toggle(led0_port, led0_mask);
            }
        }
} blinker_thread;

}  // namespace


int main() {
    nvic_init();

    nvic_set_handler(IRQ_SVCALL, vPortSVCHandler);
    nvic_set_handler(IRQ_PENDSV, xPortPendSVHandler);
    nvic_set_handler(IRQ_SYSTICK, xPortSysTickHandler);

    blinker_thread.init();

    vTaskStartScheduler();

    while (1) ;
}
