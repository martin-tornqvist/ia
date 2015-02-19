#include "sdl_wrapper.hpp"

#include <iostream>

#include <SDL_image.h>
#include <SDL_mixer.h>

#include "init.hpp"
#include "cmn_data.hpp"
#include "config.hpp"

using namespace std;

namespace sdl_wrapper
{

namespace
{

bool is_inited = false;

}

void init()
{
    TRACE_FUNC_BEGIN;

    is_inited = true;

    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
        TRACE << "Failed to init SDL" << endl;
        assert(false);
    }

    SDL_Enable_uNICODE(1);

    if (IMG_Init(IMG_INIT_PNG) == -1)
    {
        TRACE << "Failed to init SDL_image" << endl;
        assert(false);
    }

    const int     AUDIO_FREQ      = 44100;
    const Uint16  AUDIO_FORMAT    = MIX_DEFAULT_FORMAT;
    const int     AUDIO_CHANNELS  = 2;
    const int     AUDIO_BUFFERS   = 1024;

    if (Mix_Open_audio(AUDIO_FREQ, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS) == -1)
    {
        TRACE << "Failed to init SDL_mixer" << endl;
        assert(false);
    }

    Mix_Allocate_channels(AUDIO_ALLOCATED_CHANNELS);

    TRACE_FUNC_END;
}

void cleanup()
{
    is_inited = false;
    IMG_Quit();
    Mix_Allocate_channels(0);
    Mix_Close_audio();
    SDL_Quit();
}

void sleep(const Uint32 DURATION)
{
    if (is_inited && !Config::is_bot_playing())
    {
        if (DURATION == 1)
        {
            SDL_Delay(DURATION);
        }
        else
        {
            const Uint32 WAIT_UNTIL = SDL_Get_ticks() + DURATION;
            while (SDL_Get_ticks() < WAIT_UNTIL) {SDL_Pump_events();}
        }
    }
}

} //Sdl_wrapper
