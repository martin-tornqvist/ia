#include "sdl_wrapper.hpp"

#include <iostream>

#include <SDL_image.h>
#include <SDL_mixer.h>

#include "init.hpp"
#include "config.hpp"
#include "game_time.hpp"

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
        TRACE << "Failed to init SDL" << std::endl;
        ASSERT(false);
    }

    if (IMG_Init(IMG_INIT_PNG) == -1)
    {
        TRACE << "Failed to init SDL_image" << std::endl;
        ASSERT(false);
    }

    const int     AUDIO_FREQ      = 44100;
    const Uint16  AUDIO_FORMAT    = MIX_DEFAULT_FORMAT;
    const int     AUDIO_CHANNELS  = 2;
    const int     AUDIO_BUFFERS   = 1024;

    if (Mix_OpenAudio(AUDIO_FREQ, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS) == -1)
    {
        TRACE << "Failed to init SDL_mixer" << std::endl;
        ASSERT(false);
    }

    Mix_AllocateChannels(audio_allocated_channels);

    TRACE_FUNC_END;
}

void cleanup()
{
    is_inited = false;
    IMG_Quit();
    Mix_AllocateChannels(0);
    Mix_CloseAudio();
    SDL_Quit();
}

void sleep(const Uint32 DURATION)
{
    if (is_inited && !config::is_bot_playing())
    {
        if (DURATION == 1)
        {
            SDL_Delay(DURATION);
        }
        else //Duration longer than 1 ms
        {
            const Uint32 WAIT_UNTIL = SDL_GetTicks() + DURATION;

            while (SDL_GetTicks() < WAIT_UNTIL)
            {
                SDL_PumpEvents();
            }
        }
    }
}

void flush_input()
{
    SDL_PumpEvents();
}

} //sdl_wrapper
