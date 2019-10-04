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

/**
 * @file
 *
 * Generic Clock interface.
 *
 * The interface defines generic functions for accessing and manipulating clocks.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Request the clock to be initialized.
 *
 * @param clk_id Generic Clock Identifier. Implementations should define constants for specific clocks.
 *
 * @returns Zero on success, value less than zero (error code) on failure.
 */
int clk_request(int clk_id);

/**
 * @brief Set the option on the clock.
 *
 * This function is different from simple clk_request() in that it should not be used to start the clock.
 * It can be used to, say, select the parent clock, enable/disable particular outputs etc.
 * Avoid using this function for setting dividers. clk_request_rate() should be used for configuring
 * the clock's rate.
 *
 * @param clk_id Generic Clock Identifier. Implementations should define constants for specific clocks.
 * @param option Generic Option Identifier. Implementations should define constants for specific options.
 *
 * @returns Zero on success, value less than zero (error code) on failure.
 */
int clk_request_option(int clk_id, int option);

/**
 * @brief Get the clock's frequency (rate).
 *
 * @param clk_id Generic Clock Identifier. Implementations should define constants for specific clocks.
 *
 * @returns Clock frequency in Hz.
 */
unsigned int clk_get_rate(int clk_id);

/**
 * @brief Set the frequency (rate) of the clock.
 *
 * @param clk_id Generic Clock Identifier. Implementations should define constants for specific clocks.
 *
 * @returns The frequency that was actually set, which may be different from the one requested, if it's not supported.
 */
unsigned int clk_request_rate(int clk_id, unsigned int rate);

#ifdef __cplusplus
}
#endif
