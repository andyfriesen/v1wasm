#ifndef DEVICE_NACL_H
#define DEVICE_NACL_H


#include "audiere.h"
#include "device_mixer.h"


namespace audiere {

  class NaclAudioDevice : public MixerDevice {
  public:
    static NaclAudioDevice* create(const ParameterList& parameters);

  private:
    NaclAudioDevice();
    ~NaclAudioDevice();

  public:
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
