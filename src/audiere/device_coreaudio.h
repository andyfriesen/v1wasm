#ifndef DEVICE_COREAUDIO_H
#define DEVICE_COREAUDIO_H


#include "audiere.h"
#include "device_mixer.h"
#include <AudioUnit/AudioUnit.h>

namespace audiere {

  class CAAudioDevice : public MixerDevice {
  public:
    static CAAudioDevice* create(const ParameterList& parameters);

  private:
    CAAudioDevice(AudioUnit output_audio_unit);
    ~CAAudioDevice();

  public:
    void ADR_CALL update();
    const char* ADR_CALL getName();

    static OSStatus fillInput(void                        *inRefCon,
			      AudioUnitRenderActionFlags  *inActionFlags,
			      const AudioTimeStamp        *inTimeStamp,
			      UInt32                      inBusNumber,
			      UInt32                      inNumberFlags,
			      AudioBufferList             *ioData);

  private:
      AudioUnit m_output_audio_unit;
  };

}


#endif
