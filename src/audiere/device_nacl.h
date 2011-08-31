#ifndef DEVICE_NACL_H
#define DEVICE_NACL_H


#include "audiere.h"
#include "device_mixer.h"

namespace pp {
    struct Instance;
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
  };

}


#endif
