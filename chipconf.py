"""
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
"""

import os

__all__ = []

def configure_arm(env):
    if 'ARM_TOOLCHAIN_PREFIX' in os.environ:
        arm_prefix = os.environ['ARM_TOOLCHAIN_PREFIX']
    else:
        arm_prefix = 'arm-none-eabi-'

    arm_tool = lambda t: arm_prefix + t

    env.Replace(
        AR=arm_tool('ar'),
        AS=arm_tool('as'),
        CC=arm_tool('gcc'),
        CXX=arm_tool('g++'),
        LINK=arm_tool('gcc'),
        GDB=arm_tool('gdb'),
        OBJCOPY=arm_tool('objcopy'),
        OBJDUMP=arm_tool('objdump'),
        SIZE=arm_tool('size'))

def configure_chip(env, chipname):
    arch_flags = []
    arch_defines = []
    if any(chipname.lower().startswith(prefix) for prefix in 'nrf52 same5 samd5'):
        configure_arm(env)
        arch_flags = [
            '-mthumb',
            '-mcpu=cortex-m4',
            '-mfloat-abi=hard',
            '-mfpu=fpv4-sp-d16',
            ]
    elif chipname.lower().startswith('sam4s'):
        configure_arm(env)
        arch_flags = [
            '-mthumb',
            '-mcpu=cortex-m4',
            '-mfloat-abi=soft',
            ]
    elif chipname.lower() == 'nativetest':
        arch_defines=['TEST_MEMIO']

    env.AppendUnique(CCFLAGS=arch_flags)
    env.AppendUnique(LINKFLAGS=arch_flags)
    env.AppendUnique(CPPDEFINES=arch_defines)

def get_freertos_port(chip):
    if any(chip.lower().startswith(prefix) for prefix in 'nrf52 same5 samd5'):
        return 'ARM_CM4F'
    elif chip.lower().startswith('sam4s'):
        return 'ARM_CM3'
