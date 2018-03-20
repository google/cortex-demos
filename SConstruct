import os

env = Environment(
        CCFLAGS = ['-Wall', '-g', '-Wextra', '-fdiagnostics-color=always'],
        CFLAGS = ['-std=c11'],
        CXXFLAGS = ['-std=c++17'],
        CPPPATH = ['.'],
        )

CHIP = 'nrf52'
if 'DEMOS_CHIP' in os.environ:
    CHIP = os.environ['DEMOS_CHIP']

env['CHIP'] = CHIP

Export('env')

main_lib = SConscript('src/SConscript')
run_all_tests = SConscript('tests/SConscript')
