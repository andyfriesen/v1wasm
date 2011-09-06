#include <algorithm>
#include <string>
#include <stdio.h>
#include "device_nacl.h"
#include "debug.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/audio.h"

#include "dumb.h"

namespace {
    const uint32_t SAMPLE_FRAME_COUNT = 2048u;
}

namespace audiere {
  NaclAudioDevice::NaclAudioDevice(pp::Instance* instance)
    : MixerDevice(44100)
    , instance(instance)
  {
      sampleFrameCount = pp::AudioConfig::RecommendSampleFrameCount(
          PP_AUDIOSAMPLERATE_44100,
          SAMPLE_FRAME_COUNT
      );

      pp::AudioConfig audio_config = pp::AudioConfig(
          instance,
          PP_AUDIOSAMPLERATE_44100,
          sampleFrameCount
      );

      audio = new pp::Audio(
          instance,
          audio_config,
          &NaclAudioDevice::_audioCallback,
          this
      );

      audio->StartPlayback();
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


  void NaclAudioDevice::_audioCallback(void* samples, uint32_t buffer_size, void* data) {
    auto self = static_cast<NaclAudioDevice*>(data);
    self->audioCallback(samples, buffer_size);
  }


  void NaclAudioDevice::audioCallback(void* samples, uint32_t buffer_size) {
    this->read(buffer_size / sizeof(uint32_t), samples);
  }
}
