import os
import os.path
import json

def EmscriptenEnvironment():
    env = Environment()
    # env.AddMethod(Pexe, 'Pexe')

    # env['NACL_HOST'] = 'host_x86_64'
    # env['NACL_SDK_ROOT'] = '/Users/andy/src/nacl_sdk/pepper_35'
    # env['NACL_TOOLCHAIN'] = '$NACL_SDK_ROOT/toolchain/mac_pnacl/$NACL_HOST'
    # env['NACL_BIN'] = '$NACL_SDK_ROOT/toolchain/mac_pnacl/bin'
    # env['CONFIG'] = 'Debug'

    # env['NACL_ARCH'] = 'pnacl'

    emcc = '/media/andy/spinner/src/emsdk/upstream/emscripten/emcc'

    env['CC'] = emcc
    env['CXX'] = emcc

    # env.Append(LIBPATH=[
    #         '$NACL_SDK_ROOT/lib/pnacl/$CONFIG'
    #     ]
    # )

    # env.Append(CPPDEFINES=[
    #         'NACL_ARCH=$NACL_ARCH',
    #         'NACL_SDK_DEBUG',
    #         # 'USE_TR1'
    #     ]
    # )

    # env.Append(CPPPATH=[
    #         '$NACL_SDK_ROOT/include'
    #     ]
    # )

    env.Append(CXXFLAGS=[
            '-fcolor-diagnostics'
        ]
    )

    env.Append(CXXFLAGS=[
            '-O1', '-g',
            # '-O3',

            '-MMD',
            '-Wno-parentheses',
            '-Wno-long-long',
            '-Wno-dangling-else',
        ]
    )

    env.Append(LINKFLAGS=[
        '-g4', '--source-map-base', 'http://localhost:8000/',

        '-s', 'ASYNCIFY_IMPORTS=\'["fetchSync","wasm_nextFrame","emscripten_sleep"]\'',
        '-s', 'ASYNCIFY',
        '-s', 'FETCH=1',
    ])

    # env.Append(LIBS=[
    #         'ppapi',
    #         'ppapi_cpp',
    #     ]
    # )

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
    threads_posix.cpp
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
        # '-std=c++11',
        '-std=c++17',
        #'-Werror', # Hahaha no way.  This code dates back to like 1997.
    ],
    CPPDEFINES=[
        'NO_FLAC',
        'NO_SPEEX',
        'NO_OGG',
        'NO_MP3',
        'AUDIERE_NACL',
        'VERGE_AUDIO'
    ]
)

verge = env.Program('verge.out.js', sources + audiereSource)# + dumbSource)