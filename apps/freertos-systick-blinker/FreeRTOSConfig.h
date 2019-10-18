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

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION        1
#define configUSE_IDLE_HOOK         0
#define configUSE_TICK_HOOK         (0)
#define configUSE_TICKLESS_IDLE     (0)

extern unsigned int g_cpu_clock_hz;
#define configCPU_CLOCK_HZ          (g_cpu_clock_hz)

#define configTICK_RATE_HZ          ( ( TickType_t ) 100 )
// This minimal is for real.
#define configMINIMAL_STACK_SIZE    ( ( unsigned short ) 20 )
#define configTOTAL_HEAP_SIZE       ( ( size_t ) ( 19 * 1024 ) )
#define configMAX_TASK_NAME_LEN     ( 6 )
#define configUSE_TRACE_FACILITY    1
#define configUSE_16_BIT_TICKS      0
#define configIDLE_SHOULD_YIELD     0
#define configUSE_TASK_NOTIFICATIONS    (1)
#define configUSE_CO_ROUTINES       0
#define configUSE_MUTEXES           0

#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

#define configUSE_COUNTING_SEMAPHORES   0
#define configUSE_ALTERNATIVE_API       0
#define configCHECK_FOR_STACK_OVERFLOW  0
//#define configCHECK_FOR_STACK_OVERFLOW    2
#define configUSE_RECURSIVE_MUTEXES     0
#define configQUEUE_REGISTRY_SIZE       10
#define configGENERATE_RUN_TIME_STATS   0
#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 configSUPPORT_DYNAMIC_ALLOCATION
#define INCLUDE_vTaskCleanUpResources       0
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_uxTaskGetStackHighWaterMark 1

#define configPRIO_BITS     (3)
#define configMAX_PRIORITIES        ((1 << (8 - configPRIO_BITS)) - 1)

#define configUSE_STATS_FORMATTING_FUNCTIONS    1
#define configKERNEL_INTERRUPT_PRIORITY     (configMAX_PRIORITIES << configPRIO_BITS)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (8 << configPRIO_BITS)

/*-----------------------------------------------------------
 * Macros required to setup the timer for the run time stats.
 *-----------------------------------------------------------*/
// extern void vConfigureTimerForRunTimeStats( void );

#endif /* FREERTOS_CONFIG_H */
