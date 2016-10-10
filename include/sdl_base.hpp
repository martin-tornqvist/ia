#ifndef SDL_BASE_HPP
#define SDL_BASE_HPP

#include <SDL.h>

//
// TODO: This can probably be merged with the io namespace
//

namespace sdl_base
{

void init();

void cleanup();

void sleep(const Uint32 duration);

}

#endif // SDL_BASE_HPP
