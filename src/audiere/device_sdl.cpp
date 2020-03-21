#include <algorithm>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include "device_sdl.h"
#include "debug.h"

namespace audiere {
  void SDLAudioDevice::sdl_callback(void *data,  Uint8 *stream, int len)
  {
	((SDLAudioDevice *)data)->real_callback(stream, len);
  }

  void SDLAudioDevice::real_callback(Uint8 *stream, int len)
  {
//         fprintf(stderr, "sdl cbk: %d bytes needed, size:%d\n", len, audio_offset);

         if (len <= audio_offset) {
                SDL_MixAudio(stream, (const Uint8 *)audio_buffer, len, SDL_MIX_MAXVOLUME);
                
                audio_offset -= len;

                if (audio_offset > 0) {
                        memcpy(audio_buffer, (audio_buffer + len), audio_offset);
                }
                else
                        audio_offset = 0;
         }
         else {
                 if (audio_offset > 0) {
                     SDL_MixAudio(stream, (const Uint8 *)audio_buffer, audio_offset, SDL_MIX_MAXVOLUME);         
                     len -= audio_offset;
                 }

                 memset(stream + audio_offset, silence, len);
                 
                 audio_offset = 0;
         } 
  }
  
  SDLAudioDevice*
  SDLAudioDevice::create(const ParameterList& parameters) {
    std::string device = parameters.getValue("device", "DEFAULT");

    if (device != "DEFAULT") {
         device = "SDL_AUDIO_DRIVER=" + device;
         putenv(const_cast<char *>(device.c_str()));
    }
    
    SDL_AudioSpec pars;

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Unable to init SDL audio: %s.\n", SDL_GetError());
        return 0;
    }
   
//    fprintf(stderr, "SDL audio intialized\n");
    
    pars.freq = 44100;
    pars.callback = sdl_callback;
    pars.format = AUDIO_S16SYS;
    pars.channels = 2;
    pars.samples = 2048;

    SDLAudioDevice *dev =  new SDLAudioDevice(pars.samples * 16, pars.silence);
    pars.userdata = dev;
    
    if (SDL_OpenAudio(&pars, NULL) < 0) {
      delete dev;
      fprintf(stderr, "Unable to open SDL audio unit: %s.\n", SDL_GetError());
      return 0;
    }

//    fprintf(stderr, "SDL unit opened: %lx\n", dev);
    return dev;
  }

  void ADR_CALL
  SDLAudioDevice::update() {
    static const int BUFFER_SIZE = 1024; // read reads size x 4

//    fprintf(stderr, "update call: %d size\n", audio_offset);

    while (audio_offset > (0x8000 - BUFFER_SIZE * 4)) {
//            fprintf(stderr, "buffer full, waiting...\n");
             usleep(40000);
    }
    SDL_LockAudio();
    read(BUFFER_SIZE, audio_buffer + audio_offset);
    audio_offset += (BUFFER_SIZE * 4);

    if (audio_offset >= 0x4000 && !audio_started) {
            audio_started = true;
            SDL_PauseAudio(0);
    }

    SDL_UnlockAudio();
  }


  const char* ADR_CALL
  SDLAudioDevice::getName() {
    return "sdl";
  }

}
