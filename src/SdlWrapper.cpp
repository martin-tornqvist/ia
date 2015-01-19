#include "SdlWrapper.h"

#include <iostream>

#include <SDL_image.h>
#include <SDL_mixer.h>

#include "Init.h"
#include "CmnData.h"
#include "Config.h"

using namespace std;

namespace SdlWrapper
{

namespace
{

bool isInited = false;

}

void init()
{
    TRACE_FUNC_BEGIN;

    isInited = true;

    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
        TRACE << "Failed to init SDL" << endl;
        assert(false);
    }

    SDL_EnableUNICODE(1);

    if (IMG_Init(IMG_INIT_PNG) == -1)
    {
        TRACE << "Failed to init SDL_image" << endl;
        assert(false);
    }

    const int     AUDIO_FREQ      = 44100;
    const Uint16  AUDIO_FORMAT    = MIX_DEFAULT_FORMAT;
    const int     AUDIO_CHANNELS  = 2;
    const int     AUDIO_BUFFERS   = 1024;

    if (Mix_OpenAudio(AUDIO_FREQ, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS) == -1)
    {
        TRACE << "Failed to init SDL_mixer" << endl;
        assert(false);
    }

    Mix_AllocateChannels(AUDIO_ALLOCATED_CHANNELS);

    TRACE_FUNC_END;
}

void cleanup()
{
    isInited = false;
    IMG_Quit();
    Mix_AllocateChannels(0);
    Mix_CloseAudio();
    SDL_Quit();
}

void sleep(const Uint32 DURATION)
{
    if (isInited && !Config::isBotPlaying())
    {
        if (DURATION == 1)
        {
            SDL_Delay(DURATION);
        }
        else
        {
            const Uint32 WAIT_UNTIL = SDL_GetTicks() + DURATION;
            while (SDL_GetTicks() < WAIT_UNTIL) {SDL_PumpEvents();}
        }
    }
}

} //SdlWrapper
