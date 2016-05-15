#ifndef SDL_WRAPPER_HPP
#define SDL_WRAPPER_HPP

#include <SDL.h>

namespace sdl_wrapper
{

void init();

void cleanup();

void sleep(const Uint32 DURATION);

}

#endif
