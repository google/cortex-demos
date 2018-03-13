env = Environment(
        CCFLAGS = ['-Wall', '-g', '-Wextra', '-fdiagnostics-color=always'],
        CFLAGS = ['-std=c11'],
        CXXFLAGS = ['-std=c++17'],
        CPPPATH = ['.'],
        )

Export('env')

main_lib = SConscript('src/SConscript')
run_all_tests = SConscript('tests/SConscript')
