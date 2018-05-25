import os
import chipconf
import freertos

def make_chip_hwenv(tmpl_env, chip):
    hwenv = tmpl_env.Clone()
    hwenv['CHIP'] = chip
    chipconf.configure_chip(hwenv, chip)
    hwenv.AppendUnique(CPPPATH='#/src/chip-%s' % chip, CPPDEFINES=['CHIP_%s' % chip.upper()])

    return hwenv

def get_chip_apps(chip):
    return ['apps/nvic-hwtest/SConscript',
            'apps/rtc-blinker/SConscript',
            'apps/freertos-blinker/SConscript',
            'apps/saadc-basic/SConscript',
            ]

env = Environment(
        CCFLAGS = ['-Wall', '-g', '-Wundef', '-Wextra', '-Wredundant-decls',
            '-fdiagnostics-color=always'],
        CFLAGS = ['-std=c11', '-Wstrict-prototypes'],
        CXXFLAGS = ['-std=c++17'],
        CPPPATH = ['.', '#/src'],
        )

supported_chips = 'nrf52'.split()
if 'DEMOS_CHIP' in os.environ:
    CHIP = os.environ['DEMOS_CHIP']
else:
    CHIP = supported_chips[0]

# Build libraries for all supported chips
hwenv = env.Clone()
hwenv.AppendUnique(
        CCFLAGS=['-fno-common', '-ffunction-sections', '-fdata-sections'],
        CXXFLAGS=['-fno-rtti', '-fno-exceptions', '-fno-use-cxa-atexit'],
        LIBPATH=['#/src/linker-scripts'],
        LINKFLAGS=[
            '--static',
            '-nostartfiles',
            '-Wl,--start-group',
            '-lc_nano', '-lgcc',
            '-Wl,--end-group',
            '-Wl,--gc-sections',
            ]
        )

native_env = env.Clone()
native_env.AppendUnique(
        CPPPATH='#/src/chip-nativetest',
        CPPDEFINES=['TEST_MEMIO', 'CHIP_NATIVETEST'],
        )
for chip in supported_chips:
    chip_hwenv = make_chip_hwenv(hwenv, chip)
    SConscript('src/SConscript', exports=dict(hwenv=chip_hwenv, native_env=native_env))

test_env = native_env.Clone()
test_env.AppendUnique(LIBPATH='#/src')

test_lib = SConscript('tests/SConscript', exports=dict(env=test_env, supported_chips=supported_chips))

# Applications
app_test_env = test_env.Clone()
app_test_env.AppendUnique(LIBS=test_lib)

app_env = hwenv.Clone()
app_env.AppendUnique(LIBPATH='#/src', CPPPATH=freertos.includes('#/src/FreeRTOS', 'ARM_CM4F'))

for chip in supported_chips:
    chip_hwenv = make_chip_hwenv(app_env, chip)
    chip_hwenv.AppendUnique(LIBS='demos_%s' % chip.lower())
    for app_script in get_chip_apps(chip):
        SConscript(app_script, exports=dict(env=chip_hwenv, freertos_path='#/src/FreeRTOS', freertos_port='ARM_CM4F', test_env=app_test_env))
