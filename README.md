# VERGE

This is a WASM port of version 1 of Vecna's VERGE engine.

I chose it as a guinea pig to play with ~NaCl~ WASM development because it's sophisticated enough to prove some kind of challenge without being huge and unweildly.

# Compiling

Install SCons and Emscripten.

Note: Python 3 and `python3-distutils` are required by Emscripten.

```bash
$ scons
```

Then run an HTTP server rooted the root of this repository and load index.html in a browser.