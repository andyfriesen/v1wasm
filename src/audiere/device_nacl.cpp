#include <algorithm>
#include <string>
#include <stdio.h>
#include "device_nacl.h"
#include "debug.h"


namespace audiere {

  NaclAudioDevice*
  NaclAudioDevice::create(const ParameterList& parameters) {
    return new NaclAudioDevice();
  }


  NaclAudioDevice::NaclAudioDevice()
    : MixerDevice(44100)
  {
  }


  NaclAudioDevice::~NaclAudioDevice() {
    ADR_GUARD("NaclAudioDevice::~NaclAudioDevice");
  }


  void ADR_CALL
  NaclAudioDevice::update() {
  }


  const char* ADR_CALL
  NaclAudioDevice::getName() {
    return "nacl";
  }

}
