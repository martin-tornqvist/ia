#ifndef SDL_WRAPPER_HPP
#define SDL_WRAPPER_HPP

#include <SDL.h>

namespace sdl_wrapper
{

void init();

void cleanup();

void sleep(const Uint32 DURATION);

//Useful to call when e.g. loading lots of resources or performing lots of
//heavy calculations, so that the game doesn't appear totally busy, and the
//window can still be moved etc.
void flush_input();

}

#endif
