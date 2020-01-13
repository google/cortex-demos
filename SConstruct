"""
    Copyright 2018,2019 Google LLC

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
import chipconf

AddOption('--no-build-apps',
          dest='build_apps',
          action='store_false',
          default=True,
          help='Do not build default appliations')

AddOption('--no-build-firmware',
          dest='build_firmware',
          action='store_false',
          default=True,
          help='Do not build firmware libraries (i.e. only build native tests/libraries)')

AddOption('--no-debug',
          dest='with_debug',
          action='store_false',
          default=True,
          help='Disable debug symbols and optimize the code.')

AddOption('--no-chip-tests',
          dest='build_chip_tests',
          action='store_false',
          default=True,
          help='Do not build chip specific tests')

def make_chip_hwenv(tmpl_env, chip):
    hwenv = tmpl_env.Clone()
    hwenv['CHIP'] = chip
    chipconf.configure_chip(hwenv, chip)
    hwenv.AppendUnique(CPPPATH='#/src/chip-%s' % chip, CPPDEFINES=['CHIP_%s' % chip.upper()])

    return hwenv

def get_chip_apps(chip):
    if chip.startswith('nrf52'):
        return ['nvic-hwtest',
                'rtc-blinker',
                'freertos-blinker',
                'saadc-basic',
                'feather-blinker',
                ]
    elif chip.startswith('sam4s'):
        return ['nvic-hwtest',
                'freertos-systick-blinker',
                ]
    elif chip.startswith('same5'):
        return ['nvic-hwtest']

env = Environment(
        CCFLAGS = ['-Wall', '-Wundef', '-Wextra', '-Wredundant-decls',
            '-fdiagnostics-color=always'],
        CFLAGS = ['-std=c11', '-Wstrict-prototypes'],
        CXXFLAGS = ['-std=c++17'],
        CPPPATH = ['.', '#/src'],
        ENV = {'PATH': os.environ['PATH']},
        )

if GetOption('with_debug'):
  env.AppendUnique(CCFLAGS=['-g'])
else:
  env.AppendUnique(CCFLAGS=['-O2'])

env['BUILD_CHIP_TESTS'] = GetOption('build_chip_tests')

supported_chips = 'nrf52 sam4s same5'.split()

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
            '-Wl,--undefined=uxTopUsedPriority',
            ]
        )

native_env = env.Clone()
native_env.AppendUnique(
        CPPPATH='#/src/chip-nativetest',
        CPPDEFINES=['TEST_MEMIO', 'CHIP_NATIVETEST'],
        )
for chip in supported_chips:
    chip_hwenv = make_chip_hwenv(hwenv, chip)
    SConscript('src/SConscript',
               variant_dir=os.path.join('build', 'mainlib', chip),
               exports=dict(hwenv=chip_hwenv, native_env=native_env))

test_env = native_env.Clone()
test_env.AppendUnique(LIBPATH='#/src', CPPPATH=[
    '#', os.path.join('#', 'third_party', 'FreeRTOS', 'Source', 'include'),
    os.path.join('#', 'third_party', 'FreeRTOS', 'Source', 'portable', 'GCC', 'mock'),
    ])

for chip in supported_chips:
    chip_test_env = test_env.Clone()
    chip_test_env.AppendUnique(LIBPATH=[os.path.join('#', 'build', 'mainlib', chip)])
    test_lib = SConscript('tests/SConscript',
            variant_dir=os.path.join('build', 'tests', chip),
            exports=dict(env=chip_test_env, chip=chip))

# Applications
if GetOption('build_apps'):
  app_test_env = test_env.Clone()
  app_test_env.AppendUnique(LIBS=test_lib)

  app_env = hwenv.Clone()
  app_env.AppendUnique(LIBS='demos', CPPPATH=[os.path.join('#', 'third_party', 'FreeRTOS', 'Source', 'include')])

  for chip in supported_chips:
      chip_hwenv = make_chip_hwenv(app_env, chip)
      chip_hwenv.AppendUnique(LIBPATH=[os.path.join('#', 'build', 'mainlib', chip)])
      for app in get_chip_apps(chip):
          freertos_port_name = chipconf.get_freertos_port(chip)
          app_hwenv = chip_hwenv.Clone()
          app_hwenv.AppendUnique(CPPPATH=[
              os.path.join('#', 'apps', app),
              os.path.join('#', 'third_party', 'FreeRTOS', 'Source', 'portable', 'GCC', freertos_port_name),
              ])
          SConscript(os.path.join('apps', app, 'SConscript'),
                  variant_dir=os.path.join('build', 'apps', chip, app),
                  exports=dict(env=app_hwenv, freertos_path='#/third_party/FreeRTOS',
                      freertos_port=freertos_port_name, test_env=app_test_env))
