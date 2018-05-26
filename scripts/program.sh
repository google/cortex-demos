#!/bin/bash

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

if test  X${1} != X; then
    APP_NAME="${1}"
else
    APP_NAME="freertos-blinker"
fi

echo "Flashing ${APP_NAME} on board ${BOARD} (${CHIP})"

OPENOCD_PATH="${CFG_PATH}/${BOARD}"
OPENOCD_CFG="${CFG_PATH}/${BOARD}/program.cfg"
SOURCE="${SCRIPTDIR}/../build/apps/${CHIP}/${APP_NAME}/firmware.elf"

${OPENOCD} -s ${OPENOCD_PATH} -f ${OPENOCD_CFG} -c "program $SOURCE verify reset exit"
