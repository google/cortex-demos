#!/bin/bash

SCRIPTDIR=${0%/*.sh}

# TODO: More robus way to configure these variables
OPENOCD=`which openocd`
CFG_PATH="${SCRIPTDIR}/../configs/openocd"
BOARD="nrf52dk"
CHIP="nrf52"

if test  X${1} != X; then
    APP_NAME="${1}"
else
    APP_NAME="freertos-blinker"
fi

echo "Flashing ${APP_NAME}"

OPENOCD_PATH="${CFG_PATH}/${BOARD}"
OPENOCD_CFG="${CFG_PATH}/${BOARD}/program.cfg"
SOURCE="${SCRIPTDIR}/../apps/${APP_NAME}/image_${CHIP}.elf"

${OPENOCD} -s ${OPENOCD_PATH} -f ${OPENOCD_CFG} -c "program $SOURCE verify reset exit"
