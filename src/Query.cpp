#include "Query.h"

#include <SDL/SDL.h>

#include <iostream>

#include "Engine.h"

using namespace std;


void Query::waitForKeyPress() const {
  if(eng->config->BOT_PLAYING == false) {
    readKeys();
  }
}

int Query::readKeys() const {
  SDL_Event event;
  for(unsigned int i = 0; i < 1;) {
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_KEYDOWN: {
        Uint16 key = static_cast<Uint16>(event.key.keysym.sym);

        if(key == SDLK_RIGHT || key == SDLK_UP || key == SDLK_LEFT || key == SDLK_DOWN || key == SDLK_SPACE || key
            == SDLK_ESCAPE || key == SDLK_PAGEUP || key == SDLK_HOME || key == SDLK_END || key == SDLK_PAGEDOWN) {
          return key;
        } else {
          return event.key.keysym.unicode;
        }
      }
      break;
      default: {
      }
      break;
      }
    }
    SDL_Delay(1);
  }
  return 0;
}

bool Query::yesOrNo() const {
  int r = 0;
  while(r != int('y') && r != int('n') && r != SDLK_ESCAPE && r != SDLK_SPACE) {
    r = readKeys();
  }
  if(r == int('y')) {
    return true;
  }

  return false;
}

void Query::waitForEscOrSpace() const {
  if(eng->config->BOT_PLAYING == false) {
    int r = 0;
    while(r != SDLK_SPACE && r != SDLK_ESCAPE) {
      r = readKeys();
    }
  }
}

coord Query::direction() const {
  int r = 0;
  while(r != SDLK_RIGHT && r != SDLK_UP && r != SDLK_LEFT && r != SDLK_DOWN && r != SDLK_ESCAPE && r != SDLK_SPACE && r
        != SDLK_PAGEUP && r != SDLK_HOME && r != SDLK_END && r != SDLK_PAGEDOWN && (r < int('1') || r > int('9') || r
            == int('5'))) {
    r = readKeys();
  }

  const bool SHIFT = SDL_GetModState() & KMOD_SHIFT;
  const bool CTRL = SDL_GetModState() & KMOD_CTRL;

  switch(r) {
  case SDLK_SPACE:
  case SDLK_ESCAPE:
    return coord(0, 0);
    break;
  case SDLK_RIGHT:
  case int('6'): {
    if(SHIFT) {
      return coord(1, -1);
    } else {
      if(CTRL) {
        return coord(1, 1);
      } else {
        return coord(1, 0);
      }
    }
  }
  break;
  case int('9'):
  case SDLK_PAGEUP:
    return coord(1, -1);
    break;
  case SDLK_UP:
  case int('8'): {
    return coord(0, -1);
  }
  break;
  case int('7'):
  case SDLK_HOME:
    return coord(-1, -1);
    break;
  case SDLK_LEFT:
  case int('4'): {
    if(SHIFT) {
      return coord(-1, -1);
    } else {
      if(CTRL) {
        return coord(-1, 1);
      } else {
        return coord(-1, 0);
      }
    }
  }
  break;
  case int('1'):
  case SDLK_END:
    return coord(-1, 1);
    break;
  case SDLK_DOWN:
  case int('2'): {
    return coord(0, 1);
  }
  break;
  case int('3'):
  case SDLK_PAGEDOWN:
    return coord(1, 1);
    break;
  }

  return coord(0, 0);
}
