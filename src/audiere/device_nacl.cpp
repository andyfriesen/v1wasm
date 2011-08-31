#include <algorithm>
#include <string>
#include <stdio.h>
#include "device_nacl.h"
#include "debug.h"
#include "ppapi/cpp/instance.h"


namespace audiere {

  NaclAudioDevice::NaclAudioDevice(pp::Instance* instance)
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
