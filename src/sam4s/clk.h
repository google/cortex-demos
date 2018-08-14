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

#define SAM4S_CLK_PIDCK(N)       (SAM4S_CLK_PID0CK + (N))

enum clock_id {
    /* Embedded 32kHz RC Oscillator */
    SAM4S_CLK_XTAL_RC,
    /* 32768 Crystal Oscillator on XIN32 XOUT32 pins */
    SAM4S_CLK_XTAL_EXT,
    /* Embedded 4/8/12 MHz Fast RC Oscillator */
    SAM4S_CLK_FAST_RC,
    /* 3-20MHz Crystal or Ceramic Resonator Oscillator */
    SAM4S_CLK_HF_CRYSTAL,

    /* Logical Clocks derived from the ones above */
    SAM4S_CLK_SLCK,
    /* clk_request interprets the request of this clock
     * as a request to switch to Fast Crystal
     */
    SAM4S_CLK_MAINCK,
    SAM4S_CLK_PLLACK,
    SAM4S_CLK_PLLBCK,

    /* Next step abstaction, clocks fed into the system */

    /* Processor Clock */
    SAM4S_CLK_HCLK,
    SAM4S_CLK_SYSTICK,
    /* Free Running clock */
    SAM4S_CLK_FCLK,
    /* Master Clock */
    SAM4S_CLK_MCK,

    /* Common Peripherals Clock */
    SAM4S_CLK_PCK,

    /* First peripheral clock.
     * This one is actually permanently enabled.
     * The clocks that can be enabled/disabled start at ID8
     */
    SAM4S_CLK_PID0CK,
};

#ifdef __cplusplus
extern "C" {
#endif

void sam4s_set_crystal_frequency(unsigned int);

#ifdef __cplusplus
}
#endif
