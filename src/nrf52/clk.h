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

/* This file defines clock constants for NRF52 chip family. */

enum clock_id {
    NRF52_HFCLK_RC,
    NRF52_HFCLK_XTAL,
    NRF52_LFCLK_RC,
    NRF52_LFCLK_XTAL,
    NRF52_LFCLK_XTAL_EXT,
    NRF52_LFCLK_SYNTH,
};

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif
