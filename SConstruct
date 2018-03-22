import os
import chipconf

env = Environment(
        CCFLAGS = ['-Wall', '-g', '-Wundef', '-Wextra', '-Wredundant-decls',
            '-fdiagnostics-color=always'],
        CFLAGS = ['-std=c11', '-Wstrict-prototypes'],
        CXXFLAGS = ['-std=c++17'],
        CPPPATH = ['.', '#/src'],
        )

supported_chips = 'nrf52 nativetest'.split()
if 'DEMOS_CHIP' in os.environ:
    CHIP = os.environ['DEMOS_CHIP']
else:
    CHIP = supported_chips[0]

# Build libraries for all supported chips
hwenv = env.Clone()
hwenv.AppendUnique(
        CCFLAGS=['-fno-common', '-ffunction-sections', '-fdata-sections'],
        CXXFLAGS=['-fno-rtti', '-fno-exceptions'],
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
for chip in supported_chips:
    chip_hwenv = hwenv.Clone()
    chip_hwenv['CHIP'] = chip
    chipconf.configure_chip(chip_hwenv, chip)
    chip_hwenv.AppendUnique(CPPPATH='#/src/chip-%s' % chip, CPPDEFINES=['CHIP_%s' % chip.upper()])

    demos_lib = SConscript('src/SConscript', exports=dict(hwenv=chip_hwenv))
    if chip != 'nativetest':
        SConscript('apps/nvic-hwtest/SConscript', exports=dict(hwenv=chip_hwenv, demos_lib=demos_lib))

test_env = env.Clone()
test_env.AppendUnique(LIBS='demos_nativetest', CPPPATH='#/src/chip-nativetest',
        LIBPATH='#/src', CPPDEFINES=['TEST_MEMIO'])
run_all_tests = SConscript('tests/SConscript', exports=dict(env=test_env))
