CC=emcc

EMSCRIPTEN_OPTS=\
	-s ASYNCIFY                                                       		\
	-s ASYNCIFY_STACK_SIZE=16384                                            \
	-s ASYNCIFY_IMPORTS='["fetchSync","wasm_nextFrame","emscripten_sleep"]' \
	-s FETCH=1                                                              \
	-s FORCE_FILESYSTEM=1

CFLAGS=\
	-O3                                                                     \
	-g                                                                      \
	-MMD                                                                    \
	-Wno-parentheses                                                        \
	-Wno-long-long                                                          \
	-Wno-dangling-else                                                      \
	-Wno-writable-strings                                                   \
	-std=c++17

LDFLAGS=\
	-g4                                     \
	--source-map-base http://localhost:8000 \
	-lidbfs.js

	# src/andyvc.h     \

HEADERS=\
	src/base64.h     \
	src/battle.h     \
	src/control.h    \
	src/engine.h     \
	src/entity.h     \
	src/fs.h         \
	src/keyboard.h   \
	src/libfuncs.h   \
	src/main.h       \
	src/menu2.h      \
	src/menu.h       \
	src/mikmod.h     \
	src/mtypes.h     \
	src/nichgvc.h    \
	src/pcx.h        \
	src/render.h     \
	src/ricvc.h      \
	src/shared_ptr.h \
	src/sound.h      \
	src/stack.h      \
	src/timer.h      \
	src/vc.h         \
	src/vclib.h      \
	src/vga.h        \
	src/wasm.h       \
	src/wyrdvc.h     \
	src/xbigdvc.h

	# andyvc.o  \

OBJS=\
	base64.o  \
	battle.o  \
	control.o \
	engine.o  \
	entity.o  \
	fs.o      \
	main.o    \
	menu.o    \
	menu2.o   \
	nichgvc.o \
	pcx.o     \
	render.o  \
	ricvc.o   \
	sound.o   \
	stack.o   \
	timer.o   \
	vc.o      \
	vclib.o   \
	vga.o     \
	wyrdvc.o  \
	xbigdvc.o

%.o: src/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(EMSCRIPTEN_OPTS)

verge.out.js: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(EMSCRIPTEN_OPTS) $(OBJS)

.PHONY: clean

clean:
	(rm $(OBJS) $(OBJS:.o=.d) verge.out.js verge.out.wasm verge.out.wasm.map 2> /dev/null) || true
