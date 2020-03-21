#ifndef DEVICE_NACL_H
#define DEVICE_NACL_H


#include "audiere.h"
#include "device_mixer.h"

namespace audiere {

  struct WasmAudioBuffers;

  class WasmAudioDevice : public MixerDevice {
  private:
    ~WasmAudioDevice();

  public:
    WasmAudioDevice();

    // AbstractDevice::registerCallback
    // AbstractDevice::unregisterCallback
    // AbstractDevice::clearCallbacks
    // MixerDevice::openStream
    // MixerDevice::openBuffer
    void ADR_CALL update();
    const char* ADR_CALL getName();

    static void _audioCallback(void* self, WasmAudioBuffers* dest, int framesToRender);
    void audioCallback(WasmAudioBuffers* dest, int framesToRender);

  private:
    uint32_t sampleFrameCount;
    std::vector<signed short> sampleBuffer;

    std::vector<float> leftOut;
    std::vector<float> rightOut;
  };

}


#endif
