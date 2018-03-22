import os
import chipconf

env = Environment(
        CCFLAGS = ['-Wall', '-g', '-Wundef', '-Wextra',
            '-Wstrict-prototypes', '-Wextra',
            '-fdiagnostics-color=always'],
        CFLAGS = ['-std=c11'],
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
hwenv.AppendUnique(CXXFLAGS=['-fno-rtti', '-fno-exceptions'])
for chip in supported_chips:
    chip_hwenv = hwenv.Clone()
    chip_hwenv['CHIP'] = chip
    chipconf.configure_chip(chip_hwenv, chip)
    chip_hwenv.AppendUnique(CPPPATH='#/src/chip-%s' % chip)

    demos_lib = SConscript('src/SConscript', exports=dict(hwenv=chip_hwenv))

SConscript('apps/nvic-hwtest/SConscript', exports=['hwenv', 'demos_lib'])

test_env = env.Clone()
test_env.AppendUnique(LIBS='demos_nativetest', CPPPATH='#/src/chip-nativetest',
        LIBPATH='#/src', CPPDEFINES=['TEST_MEMIO'])
run_all_tests = SConscript('tests/SConscript', exports=dict(env=test_env))
