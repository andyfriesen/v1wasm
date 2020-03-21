#ifndef DEVICE_PULSE_H
#define DEVICE_PULSE_H

#include <pulse/simple.h>
#include "audiere.h"
#include "device_mixer.h"

namespace audiere {

  class PulseAudioDevice : public MixerDevice {
  public:
    static PulseAudioDevice* create(const ParameterList& parameters);

  private:
    PulseAudioDevice(std::string& description);
    ~PulseAudioDevice();

  public:
    void ADR_CALL update();
    const char* ADR_CALL getName();

  private:
    struct pa_simple *m_sp;
    struct pa_sample_spec m_ss;
  };

}


#endif
