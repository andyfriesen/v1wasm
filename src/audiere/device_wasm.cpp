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

    try {

    const leftInPtr   = HEAPU32[device.inputPtr >> 2 + 0];
    const leftInSize  = HEAPU32[device.inputPtr >> 2 + 1];
    // const rightInPtr  = HEAPU32[device.inputPtr >> 2 + 2];
    // const rightInSize = HEAPU32[device.inputPtr >> 2 + 3];

    const leftSourceData = HEAPF32.subarray(leftInPtr >> 2, leftInPtr >> 2 + leftInSize);
    // const rightSourceData = HEAPF32.subarray(rigthInPtr >> 2, rightInPtr >> 2 + rightInSize);

    outputBuffer.copyToChannel(leftSourceData, 0, 0);
    } catch (e) {
      console.exception('NO', e);
      throw e;
    }

    // copy leftInPtr into left and rightInPtr into right
    // clear everything after the end of l and r.  Fill with 0s.

  };
  device.scriptNode.connect(device.audioContext.destination);
});

EM_JS(void, audiere_deleteDevice, (void* devicePtr), {
    
});

namespace audiere {
  WasmAudioDevice::WasmAudioDevice()
    : MixerDevice(44100)
  {
    //   sampleFrameCount = pp::AudioConfig::RecommendSampleFrameCount(
    //       instance,
    //       PP_AUDIOSAMPLERATE_44100,
    //       SAMPLE_FRAME_COUNT
    //   );

    //   pp::AudioConfig audio_config = pp::AudioConfig(
    //       instance,
    //       PP_AUDIOSAMPLERATE_44100,
    //       sampleFrameCount
    //   );

    //   audio = new pp::Audio(
    //       instance,
    //       audio_config,
    //       &WasmAudioDevice::_audioCallback,
    //       this
    //   );

    //   audio->StartPlayback();
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
      int leftSize;
      float* right;
      int rightSize;
  };

  void WasmAudioDevice::_audioCallback(void* self, WasmAudioBuffers* dest, int framesToRender) {
    WasmAudioDevice* self_ = static_cast<WasmAudioDevice*>(self);
    self_->audioCallback(dest, framesToRender);
  }


  void WasmAudioDevice::audioCallback(WasmAudioBuffers* dest, int framesToRender) {
      // TODO: _somewhere_ we need to allocate a block of memory, call this->read to fill it with samples,
      // and then set the properties of dest to point into that memory.  std::vector<float> should be fine?
    if (framesToRender > sampleBuffer.size()) {
      printf("resize %d\n", framesToRender);
      sampleBuffer.resize(framesToRender * 2);
    }

    this->read(framesToRender, sampleBuffer.data());

    dest->left = sampleBuffer.data();
    dest->leftSize = sampleBuffer.size();
  }
}
