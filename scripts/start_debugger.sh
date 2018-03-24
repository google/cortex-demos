#!/bin/bash

SCRIPTDIR=${0%/*.sh}

# TODO: More robus way to configure these variables
OPENOCD=`which openocd`
CFG_PATH="${SCRIPTDIR}/../configs/openocd"
BOARD="nrf52dk"
CHIP="nrf52"

OPENOCD_PATH="${CFG_PATH}/${BOARD}"
OPENOCD_CFG="${CFG_PATH}/${BOARD}/debug.cfg"

${OPENOCD} -s ${OPENOCD_PATH} -f ${OPENOCD_CFG}
