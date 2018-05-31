#pragma once

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
};
