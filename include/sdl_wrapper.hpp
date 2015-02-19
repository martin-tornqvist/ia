#ifndef SDL_WRAPPER_H
#define SDL_WRAPPER_H

#include <SDL.h>

namespace sdl_wrapper
{

void init();

void cleanup();

void sleep(const Uint32 DURATION);

}

#endif
