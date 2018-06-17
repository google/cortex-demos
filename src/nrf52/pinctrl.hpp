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

namespace pinctrl {

enum saadc {
    NC = 0,
    AIN0,
    AIN1,
    AIN2,
    AIN3,
    AIN4,
    AIN5,
    AIN6,
    AIN7,
    VDD,
    VDDHDIV5 = 0x11,
};

enum function {
    UARTE_RTS = 1,
    UARTE_TXD = 2,
    UARTE_CTS = 3,
    UARTE_RXD = 4,
    SAADC_CHAN_POS = 1,
    SAADC_CHAN_NEG = 2,
    TWIM_SCL = 1,
    TWIM_SDA = 2,

    MIN_PIN_FUNCTION,
    UARTE0_GROUP,
    UARTE0_RTS,
    UARTE0_TXD,
    UARTE0_CTS,
    UARTE0_RXD,
    UARTE1_GROUP,
    UARTE1_RTS,
    UARTE1_TXD,
    UARTE1_CTS,
    UARTE1_RXD,

    SAADC_CHAN0_GROUP,
    SAADC_CHAN0_POS,
    SAADC_CHAN0_NEG,

    SAADC_CHAN1_GROUP,
    SAADC_CHAN1_POS,
    SAADC_CHAN1_NEG,

    SAADC_CHAN2_GROUP,
    SAADC_CHAN2_POS,
    SAADC_CHAN2_NEG,

    SAADC_CHAN3_GROUP,
    SAADC_CHAN3_POS,
    SAADC_CHAN3_NEG,

    SAADC_CHAN4_GROUP,
    SAADC_CHAN4_POS,
    SAADC_CHAN4_NEG,

    SAADC_CHAN5_GROUP,
    SAADC_CHAN5_POS,
    SAADC_CHAN5_NEG,

    SAADC_CHAN6_GROUP,
    SAADC_CHAN6_POS,
    SAADC_CHAN6_NEG,

    SAADC_CHAN7_GROUP,
    SAADC_CHAN7_POS,
    SAADC_CHAN7_NEG,

    SAADC_CHAN_LAST_GROUP = SAADC_CHAN7_GROUP,

    TWIM0_GROUP,
    TWIM0_SCL,
    TWIM0_SDA,

    TWIM1_GROUP,
    TWIM1_SCL,
    TWIM1_SDA,

    MAX_PIN_FUNCTION,
};

}  // namespace pinctrl
