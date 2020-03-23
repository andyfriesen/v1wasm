#include <algorithm>
#include <string>
#include <stdio.h>
#include <emscripten.h>

#include "device_wasm.h"
#include "debug.h"

namespace {
  const uint32_t SAMPLE_FRAME_COUNT = 2048u;
}

using CB = void(*)(void*, audiere::WasmAudioBuffers*, int);

EM_JS(void, audiere_createDevice, (void* devicePtr, CB process), {
  const device = {};
  window.audiereDevices = window.audiereDevices || {};
  window.audiereDevices[devicePtr] = device;

  device.maxFramesPerChunk = 4096;
  device.audioContext = new AudioContext();
  device.scriptNode = device.audioContext.createScriptProcessor(device.maxFramesPerChunk);
  device.inputPtr = Module._malloc(4 * 4); // leftPtr, leftSize, rightPtr, rightSize
  device.scriptNode.onaudioprocess = ev => {
    const outputBuffer = ev.outputBuffer;
    const framesToRender = outputBuffer.length;

    Module.dynCall_viii(process, devicePtr, device.inputPtr, framesToRender);

    const leftInPtr   = HEAPU32[(device.inputPtr >> 2) + 0];
    const leftInSize  = HEAPU32[(device.inputPtr >> 2) + 1];
    const rightInPtr  = HEAPU32[(device.inputPtr >> 2) + 2];
    const rightInSize = HEAPU32[(device.inputPtr >> 2) + 3];

    const leftSourceData = HEAPF32.subarray(leftInPtr >> 2, (leftInPtr >> 2) + leftInSize);
    const rightSourceData = HEAPF32.subarray(rightInPtr >> 2, (rightInPtr >> 2) + rightInSize);

    outputBuffer.copyToChannel(leftSourceData, 0, 0);
    outputBuffer.copyToChannel(rightSourceData, 1, 0);
  };
  device.scriptNode.connect(device.audioContext.destination);

  device.resume = () => device.audioContext.resume();

  window.addEventListener('click', device.resume);
  window.addEventListener('keydown', device.resume);
});

EM_JS(void, audiere_deleteDevice, (void* devicePtr), {
  if (!window.audiereDevices) {
    console.error('audiere_deleteDevice before audiere_createDevice?', devicePtr);
    return;
  }

  const device = window.audiereDevices[devicePtr];
  if (!device) {
    console.error('audiere_deleteDevice got a bad pointer', devicePtr);
    return;
  }

  device.scriptNode.disconnect();
  _free(device.inputPtr);
  window.removeEventListener('click', device.resume);
  window.removeEventListener('keydown', device.resume);

  delete window.audiereDevices[devicePtr];
});

namespace audiere {
  WasmAudioDevice::WasmAudioDevice()
    : MixerDevice(44100)
  {
    sampleBuffer.resize(8192);
    leftOut.resize(4096);
    rightOut.resize(4096);
    audiere_createDevice((void*)this, &WasmAudioDevice::_audioCallback);
  }


  WasmAudioDevice::~WasmAudioDevice() {
    ADR_GUARD("WasmAudioDevice::~WasmAudioDevice");
    audiere_deleteDevice(this);
  }


  void ADR_CALL
  WasmAudioDevice::update() {
  }


  const char* ADR_CALL
  WasmAudioDevice::getName() {
    return "nacl";
  }

  struct WasmAudioBuffers {
      float* left;
      intptr_t leftSize;
      float* right;
      intptr_t rightSize;
  } __attribute__((packed));

  void WasmAudioDevice::_audioCallback(void* self, WasmAudioBuffers* dest, int framesToRender) {
    WasmAudioDevice* self_ = static_cast<WasmAudioDevice*>(self);
    self_->audioCallback(dest, framesToRender);
  }


  void WasmAudioDevice::audioCallback(WasmAudioBuffers* dest, int framesToRender) {
    if (framesToRender * 2 > sampleBuffer.size()) {
      sampleBuffer.resize(framesToRender * 2);
      leftOut.resize(framesToRender);
      rightOut.resize(framesToRender);
    }

    this->read(framesToRender, sampleBuffer.data());

    // Deinterlace and convert from signed short to float32
    for (int i = 0; i < framesToRender; ++i) {
      leftOut[i]  = float(sampleBuffer[i * 2]) / 32768;
      rightOut[i] = float(sampleBuffer[i * 2 + 1]) / 32768;
    }

    dest->left = leftOut.data();
    dest->leftSize = framesToRender;
    dest->right = rightOut.data();
    dest->rightSize = framesToRender;
  }
}
