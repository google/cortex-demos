#!/bin/bash

# Copyright 2018 Google LLC

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

SCRIPTDIR=${0%/*.sh}

# TODO: More robus way to configure these variables
OPENOCD=`which openocd`
CFG_PATH="${SCRIPTDIR}/../configs/openocd"

if test X${BOARD} = X; then
    BOARD="nrf52dk"
fi

if test X${CHIP} = X; then
    CHIP="nrf52"
fi

OPENOCD_PATH="${CFG_PATH}/${BOARD}"
OPENOCD_CFG="${CFG_PATH}/${BOARD}/debug.cfg"

${OPENOCD} -s ${OPENOCD_PATH} -f ${OPENOCD_CFG}
