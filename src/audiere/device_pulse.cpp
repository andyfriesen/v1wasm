#include <algorithm>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <pulse/simple.h>
#include "device_pulse.h"
#include "debug.h"

namespace audiere {

  PulseAudioDevice*
  PulseAudioDevice::create(const ParameterList& parameters) {
    std::string description = parameters.getValue("device", "audiere playback");
    return new PulseAudioDevice(description);
  }


  PulseAudioDevice::PulseAudioDevice(std::string& description)
    : MixerDevice(44100)
  {
  
	m_ss.format = PA_SAMPLE_S16LE;
	m_ss.rate = 44100;
	m_ss.channels = 2;
	
	m_sp = pa_simple_new(
		NULL,					// default server
		"sustainer",			// name of the application
		PA_STREAM_PLAYBACK,
		NULL,					// default device
		description.c_str(),	// description of the stream
		&m_ss,					// sample format
		NULL,					// default channel map
		NULL,					// default buffering attributes,
		NULL					// ignore error code
	);
  }


  PulseAudioDevice::~PulseAudioDevice() {
    ADR_GUARD("PulseAudioDevice::~PulseAudioDevice");
    if (m_sp)
	    pa_simple_free(m_sp);
  }


  void ADR_CALL
  PulseAudioDevice::update() {
    static const int BUFFER_SIZE = 512;
    int bytes = BUFFER_SIZE * 4;
    char buffer[bytes];
    read(BUFFER_SIZE, buffer);
    
    int error;
    pa_simple_write(m_sp, buffer, bytes, &error);
  }


  const char* ADR_CALL
  PulseAudioDevice::getName() {
    return "pulse";
  }

}





