#ifndef SDL_WRAPPER_H
#define SDL_WRAPPER_H

#include <SDL.h>

namespace SdlWrapper {

void init();

void cleanup();

void sleep(const Uint32 DURATION);

}

#endif
