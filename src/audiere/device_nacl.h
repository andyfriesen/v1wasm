#ifndef DEVICE_NACL_H
#define DEVICE_NACL_H


#include "audiere.h"
#include "device_mixer.h"

namespace pp {
  struct Instance;
  struct Audio;
}

namespace audiere {

  class NaclAudioDevice : public MixerDevice {
  private:
    ~NaclAudioDevice();

  public:
    NaclAudioDevice(pp::Instance* instance);

    // AbstractDevice::registerCallback
    // AbstractDevice::unregisterCallback
    // AbstractDevice::clearCallbacks
    // MixerDevice::openStream
    // MixerDevice::openBuffer
    void ADR_CALL update();
    const char* ADR_CALL getName();

    static void _audioCallback(void* samples, uint32_t buffer_size, void* data);
    void audioCallback(void* samples, uint32_t buffer_size);

  private:
    pp::Instance* instance;
    pp::Audio* audio;

    uint32_t sampleFrameCount;
  };

}


#endif
