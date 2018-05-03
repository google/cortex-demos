import os.path

def sources(base, port, mem_mang=None):
    core_src_names = [
        'croutine.c',
        'event_groups.c',
        'list.c',
        'queue.c',
        'stream_buffer.c',
        'tasks.c',
        'timers.c']

    core_srcs = [os.path.join(base, 'Source', f) for f in core_src_names]

    port_path = os.path.join(base, 'Source', 'portable', 'GCC', port)
    port_candidates = ['port.c']
    for src in port_candidates:
        src_path = os.path.join(port_path, src)
        core_srcs.append(src_path)

    if mem_mang is not None:
        core_srcs.append(os.path.join(base, 'Source', 'portable', 'MemMang', 'heap_%s.c' % mem_mang))

    return core_srcs


def includes(base, port):
    return [
            os.path.join(base, 'Source', 'include'),
            os.path.join(base, 'Source', 'portable', 'GCC', port)
            ]


def build_lib(scons_env, base, port, mem_mang=None, suffix=None):
    objects = list()
    replacement = '.o'
    if suffix:
        replacement = '_%s.o' % suffix

    for src in sources(base, port, mem_mang):
        target_name = src.replace('.c', replacement)
        objects.append(scons_env.Object(target=target_name, source=src))

    libname = 'freertos'
    if suffix:
        libname = '%s_%s' % (libname, suffix)
    return scons_env.StaticLibrary(target=libname, source=objects)
