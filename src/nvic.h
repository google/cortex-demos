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

#pragma once

#include <stddef.h>

enum {
    IRQ_OFFSET = -16,
    IRQ_NMI = -14,
    IRQ_HARD_FAULT = -13,
    IRQ_MEMMANG_FAULT = -12,
    IRQ_BUS_FAULT = -11,
    IRQ_USAGE_FAULT = -10,
    IRQ_SVCALL = -5,
    IRQ_PENDSV = -2,
    IRQ_SYSTICK = -1,
    IRQ_IRQ0 = 0,
};

typedef void (*irq_handler_func_t)(void);

#ifdef __cplusplus
extern "C" {
#endif

void nvic_init(void);

int nvic_set_handler(int irqn, irq_handler_func_t handler_func);
int nvic_dispatch(int irqn);

/**
 *
 * @brief Set Off given interrupt
 *
 * @param[irqn] Interrupt number
 *
 * @returns 0 on success, <0 on error
 */
int nvic_irqset(int irqn);

void nvic_enable_irqs(void);
void nvic_disable_irqs(void);

void nvic_enable_irq(int irqn);
void nvic_disable_irq(int irqn);


/**
 * @brief Get interrupt vector table
 *
 * This should only be used for debugging in tests.
 */
irq_handler_func_t* nvic_get_table(void);


#ifdef __cplusplus
}  /* extern "C" */
#endif
