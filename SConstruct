import os
import os.path
import sys

# Find emcc
emcc = None
path = os.environ['PATH'].split(os.path.pathsep)
for p in path:
    p2 = os.path.join(p, 'emcc')
    if os.path.exists(p2):
        emcc = p2
        break

if not emcc:
    print('Could not find emcc.  Check your PATH?')
    sys.exit(1)

def EmscriptenEnvironment():
    if sys.platform in ('windows', 'win32'):
        env_dict = {
            'path': os.environ['PATH']
        }

        for key in ['HOME', 'USERPROFILE', 'EM_CONFIG', 'EMSDK_PYTHON', 'EMSDK_NODE']:
            value = os.environ.get(key)
            if value is not None:
                env_dict[key] = value

        env = Environment(ENV=env_dict, TOOLS=['mingw'])
    else:
        env = Environment()

    env['CC'] = emcc
    env['CXX'] = emcc

    emscriptenOpts = [
        '-s', 'ASYNCIFY',
        '-s', 'ASYNCIFY_IMPORTS=["fetchSync","downloadAll","wasm_nextFrame","emscripten_sleep"]',
        '-s', 'FETCH=1',
        '-s', 'FORCE_FILESYSTEM=1',
        '-s', 'ALLOW_MEMORY_GROWTH=1',
        '-s', 'STACK_SIZE=5MB',
    ]

    cflags = ['-fcolor-diagnostics']

    asmjs = ARGUMENTS.get('asmjs', 0)
    debug = int(ARGUMENTS.get('debug', 0))
    asan = ARGUMENTS.get('asan', 0)
    ubsan = ARGUMENTS.get('ubsan', 0)

    if debug:
        if not asan:
            emscriptenOpts += [
                '-s', 'SAFE_HEAP=1',
            ]

        emscriptenOpts += [
            '-s', 'ASYNCIFY_STACK_SIZE=327680',
            '-s', 'ASSERTIONS=1',
            '-s', 'STACK_OVERFLOW_CHECK=1',
            '-s', 'DEMANGLE_SUPPORT=1',
            '-s', 'STACK_SIZE=5MB',
        ]

        cflags.append('-g')

        env.Append(LINKFLAGS=[
            '-g',
        ])

    else:
        emscriptenOpts += [
            '-s', 'ASYNCIFY_STACK_SIZE=32768',
        ]
        cflags.append('-O3')

    if asmjs:
        env.Append(LINKFLAGS=[
            '-s', 'WASM=0',
        ])

    if asan:
        cflags.append('-fsanitize=address')
        env.Append(LINKFLAGS=['-fsanitize=address'])
    if ubsan:
        cflags.append('-fsanitize=undefined')
        env.Append(LINKFLAGS=['-fsanitize=undefined'])

    cflags.extend([
        '-MMD',
        '-Wno-parentheses',
        '-Wno-long-long',
        '-Wno-dangling-else',
        '-Wno-writable-strings',
    ])

    cflags.extend(emscriptenOpts)

    env.Append(CFLAGS=cflags)
    env.Append(CXXFLAGS=cflags)

    env.Append(LINKFLAGS=[
        '-lidbfs.js',
    ] + emscriptenOpts)

    return env

sources = Split("""
    engine.cpp  menu.cpp    nichgvc.cpp ricvc.cpp   vc.cpp      xbigdvc.cpp
    battle.cpp  entity.cpp  menu2.cpp   pcx.cpp     sound.cpp   vclib.cpp
    control.cpp main.cpp    render.cpp  timer.cpp   vga.cpp     wyrdvc.cpp

    fs.cpp      stack.cpp   base64.cpp
""")
sources = ['src/' + s for s in sources]

env = EmscriptenEnvironment()

env.Append(
    CXXFLAGS=[
        '-std=c++17',
        #'-Werror', # Hahaha no way.  This code dates back to like 1997.
    ],
)

verge = env.Program('verge.out.js', sources, PROGSUFFIX='.js')
