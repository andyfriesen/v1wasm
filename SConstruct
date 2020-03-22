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
    env = Environment()

    env['CC'] = emcc
    env['CXX'] = emcc

    env.Append(CXXFLAGS=[
            '-fcolor-diagnostics'
        ]
    )

    emscriptenOpts = [
        '-s', 'ASYNCIFY',
        '-s', 'ASYNCIFY_STACK_SIZE=32768',
        '-s', 'ASYNCIFY_IMPORTS=\'["fetchSync","downloadAll","wasm_nextFrame","emscripten_sleep"]\'',
        '-s', 'FETCH=1',
        '-s', 'FORCE_FILESYSTEM=1',
        '-s', 'ALLOW_MEMORY_GROWTH=1',
    ]

    if ARGUMENTS.get('debug', 0):
        emscriptenOpts += [
            '-s', 'SAFE_HEAP=1',
            '-s', 'ASSERTIONS=1',
            '-s', 'STACK_OVERFLOW_CHECK=1',
            '-s', 'DEMANGLE_SUPPORT=1',
        ]

        env.Append(CXXFLAGS=[
            '-g',
        ])

        env.Append(LINKFLAGS=[
            '-g4',
            '--source-map-base', 'http://localhost:8000/',
        ])

    else:
        env.Append(CXXFLAGS=[
            '-O3'
        ])

    env.Append(CXXFLAGS=[
            '-MMD',
            '-Wno-parentheses',
            '-Wno-long-long',
            '-Wno-dangling-else',
            '-Wno-writable-strings',
        ] + emscriptenOpts
    )

    env.Append(LINKFLAGS=[
        '-lidbfs.js',
    ] + emscriptenOpts)

    return env

dumbSource = ['src/dumb/' + s for s in Split("""
    core/atexit.c
    core/duhlen.c
    core/duhtag.c
    core/dumbfile.c
    core/loadduh.c
    core/makeduh.c
    core/rawsig.c
    core/readduh.c
    core/register.c
    core/rendduh.c
    core/rendsig.c
    core/unload.c
    helpers/clickrem.c
    helpers/memfile.c
    helpers/resample.c
    helpers/sampbuf.c
    helpers/silence.c
    helpers/stdfile.c
    it/itload.c
    it/itload2.c
    it/itmisc.c
    it/itorder.c
    it/itread.c
    it/itread2.c
    it/itrender.c
    it/itunload.c
    it/loadmod.c
    it/loadmod2.c
    it/loads3m.c
    it/loads3m2.c
    it/loadxm.c
    it/loadxm2.c
    it/readmod.c
    it/readmod2.c
    it/reads3m.c
    it/reads3m2.c
    it/readxm.c
    it/readxm2.c
    it/xmeffect.c
""")]

audiereSource = ['src/audiere/' + s for s in Split("""
    basic_source.cpp
    cd_null.cpp
    debug.cpp
    device.cpp
    device_mixer.cpp
    device_null.cpp
    device_wasm.cpp
    dumb_resample.cpp
    file_ansi.cpp
    input.cpp
    input_aiff.cpp
    input_mod.cpp
    input_wav.cpp
    loop_point_source.cpp
    memory_file.cpp
    midi_null.cpp
    mpaudec/bits.c
    mpaudec/mpaudec.c
    noise.cpp
    resampler.cpp
    sample_buffer.cpp
    sound.cpp
    sound_effect.cpp
    square_wave.cpp
    threads_nope.cpp
    timer_posix.cpp
    tone.cpp
    utility.cpp
    version.cpp
""")]

sources = Split("""
    andyvc.cpp  engine.cpp  menu.cpp    nichgvc.cpp ricvc.cpp   vc.cpp      xbigdvc.cpp
    battle.cpp  entity.cpp  menu2.cpp   pcx.cpp     sound.cpp   vclib.cpp
    control.cpp main.cpp    render.cpp  timer.cpp   vga.cpp

    fs.cpp      stack.cpp   base64.cpp
""")
sources = ['src/' + s for s in sources]

env = EmscriptenEnvironment()

env.Append(
    CPPPATH=[
        'src/dumb/include'
    ],
    CXXFLAGS=[
        '-std=c++17',
        #'-Werror', # Hahaha no way.  This code dates back to like 1997.
    ],
)

audiereEnv = env.Clone()

audiereEnv.Append(
    CPPDEFINES=[
        'NO_FLAC',
        'NO_SPEEX',
        'NO_OGG',
        'NO_MP3',
    ]
)

audiere = audiereEnv.Object(audiereSource + dumbSource)

verge = env.Program('verge.out.js', sources + audiere)
