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

#ifdef __cplusplus
extern "C" {
#endif

int clk_request(int clk_id);
int clk_request_option(int clk_id, int option);
unsigned int clk_get_rate(int clk_id);
unsigned int clk_request_rate(int clk_id, unsigned int rate);

#ifdef __cplusplus
}
#endif
