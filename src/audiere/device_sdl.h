#ifndef DEVICE_SDL_H
#define DEVICE_SDL_H


#include "audiere.h"
#include "device_mixer.h"
#include "SDL.h"


namespace audiere {

  class SDLAudioDevice : public MixerDevice {
  public:
    static SDLAudioDevice* create(const ParameterList& parameters);

  private:
    Uint8 silence;
    bool audio_started;
    char *audio_buffer;
    static void sdl_callback(void *data, Uint8 *stream, int len); 
    int audio_offset;

    void real_callback(Uint8 *stream, int len);

    SDLAudioDevice(int buffersize, Uint8 sil) : 
		MixerDevice(44100), silence(sil), audio_started(false),
		audio_offset(0)  
        { 
          audio_buffer = new char[buffersize]; 
        };
    
    ~SDLAudioDevice() { SDL_CloseAudio(); delete [] audio_buffer; };

  public:
    void ADR_CALL update();
    const char* ADR_CALL getName();

  };

}


#endif
