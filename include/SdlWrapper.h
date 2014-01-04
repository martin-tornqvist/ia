#ifndef SDL_WRAPPER_H
#define SDL_WRAPPER_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

class SdlWrapper {
public:
  SdlWrapper() {
    trace << "SdlWrapper::initSdl()..." << endl;

    if(SDL_Init(SDL_INIT_EVERYTHING) == -1) {
      trace << "[WARNING] Problem to init SDL, ";
      trace << "in SdlWrapper::initSdl()" << endl;
    }

    SDL_EnableUNICODE(1);

    if(IMG_Init(IMG_INIT_PNG) == -1) {
      trace << "[WARNING] Problem to init SDL_image";
      trace << ", in SdlWrapper::initSdl()" << endl;
    }

    const int     AUDIO_FREQ      = 44100;
    const Uint16  AUDIO_FORMAT    = MIX_DEFAULT_FORMAT;
    const int     AUDIO_CHANNELS  = 2;
    const int     AUDIO_BUFFERS   = 1024;

    if(Mix_OpenAudio(
          AUDIO_FREQ, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS) == -1) {
      trace << "[WARNING] Problem to init SDL_mixer";
      trace << ", in SdlWrapper::initSdl()" << endl;
    }

    Mix_AllocateChannels(AUDIO_ALLOCATED_CHANNELS);

    trace << "SdlWrapper::initSdl() [DONE]" << endl;
  }

  ~SdlWrapper() {
    IMG_Quit();
    Mix_AllocateChannels(0);
    Mix_CloseAudio();
    SDL_Quit();
  }

  inline void sleep(const Uint32 DURATION) const {
    const Uint32 TICKS_BEFORE = SDL_GetTicks();
    while(SDL_GetTicks() < TICKS_BEFORE + DURATION) {
      SDL_PumpEvents();
    }
  }

private:

};

#endif
